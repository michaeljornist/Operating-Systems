#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "message_slot.h"
#include <linux/errno.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Message Slot Kernel Module");

static message_slot *slots[MAX_SLOTS];

//Find a channel in given slot 
//Return the struct message_channel if exists , othewise NULL
message_channel* find_channel(message_slot *slot, unsigned int id) {
    message_channel *curr_channel = slot->channels;
    while (curr_channel != NULL) {
        if (curr_channel->id == id) {
            return curr_channel;
        }
        curr_channel = curr_channel->next;
    }
    return NULL;
}
//Creates a channel with given ID
//Return the struct message_channel if created , otherwise NULL
message_channel* create_channel(unsigned int id) {
    message_channel *channel = kmalloc(sizeof(message_channel), GFP_KERNEL);
    if (channel == NULL) {
        return NULL;
    }
    channel->id = id;
    channel->message_len = 0;
    channel->next = NULL;
    return channel;
}

// *******************  OPEN  *******************
static int device_open(struct inode *inode, struct file *file) {
    int minor = iminor(inode);
    slot_data *data;

    if (minor >= MAX_SLOTS) { //if the minor is bigger then 256 thats an error by the instructions
        return -EINVAL;
    }
    //Allocating a slot to the current minor if it not allocated.
    if (slots[minor] == NULL) {
        slots[minor] = kmalloc(sizeof(message_slot), GFP_KERNEL);
        if (slots[minor] == NULL) {
            return -ENOMEM;
        }
        memset(slots[minor], 0, sizeof(message_slot));
        slots[minor]->minor = minor;
        slots[minor]->total_channels = 0;
    }
    // slot_data is a struct that will be attached to private_Data in the struct file and carring a crusial data such slot pointer and channel id.
    data = kmalloc(sizeof(slot_data), GFP_KERNEL);
    if (data == NULL) {
        return -ENOMEM;
    }
    //Init data
    data->slot = slots[minor];
    data->channel_id = 0;

    //attaching to the private_data pointer in file struct
    file->private_data = data;
    return 0;
}
// ******************* end of OPEN  *******************

// *******************  RELEASE  *******************
//As mentioned in the Forum , would not release the hole slot (the channels) becuase we want a accesss to them if we reopen the device.
static int device_release(struct inode *inode, struct file *file) {
    slot_data *data = file->private_data;
    // only releasing the current data that the file struct saved (pointer to the slot and the channel id)
    if (data) {
        kfree(data);
    }

    return 0;
}
// *******************  end of RELEASE  *******************


// *******************  READ  *******************

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    slot_data *data = file->private_data;
    message_slot *slot = data->slot;
    unsigned int channel_id = data->channel_id;
    message_channel *channel;

    if (channel_id == 0) { //Invalid (by instructions) 
        return -EINVAL;
    }

    channel = find_channel(slot, channel_id);
    if (channel == NULL || channel->message_len == 0) { // If channel or message not exist.
        return -EWOULDBLOCK;
    }

    if (length < channel->message_len) { // We Read the hole massage , therefore if the massage we want to read is shorter then the massage in the channel - its an error
        return -ENOSPC;
    }
    //Kernel method to copy data safely from kernel to user.
    if (copy_to_user(buffer, channel->message, channel->message_len)) {
        return -EFAULT;
    }

    return channel->message_len;// As required.
}
// *******************  end of READ  *******************



// *******************   WRITE  *******************

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
    slot_data *data = file->private_data;
    message_slot *slot = data->slot;
    unsigned int channel_id = data->channel_id;
    message_channel *channel;

    if (channel_id == 0) { // Cant write to channel id 0.
        return -EINVAL;
    }

    if (length == 0 || length > BUF_LEN) { // if tring to write an empy massage or too big one. 
        return -EMSGSIZE;
    }

    channel = find_channel(slot, channel_id);
    if (channel == NULL) { // if the channel we want to write to not exist then we create one.
        if(slot->total_channels >= (1<<20)){
            return -ENOMEM;
        }
        channel = create_channel(channel_id);
        slot->total_channels++; //updating the size of the channels.

        if (channel == NULL) {
            return -ENOMEM;
        }
        //moving the current channel to the head of the list.
        channel->next = slot->channels;
        slot->channels = channel;
    }
    //Kernel method to copy safely from user.
    if (copy_from_user(channel->message, buffer, length)) {
        return -EFAULT;
    }

    channel->message_len = length;
    return length;
}
// *******************  end of WRITE  *******************


// *******************   IOCTL  *******************
static long device_ioctl(struct file *file, unsigned int ioctl_command_id, unsigned long ioctl_param) {
    slot_data *data = file->private_data;

    if (ioctl_command_id != MSG_SLOT_CHANNEL || ioctl_param == 0) { //If the channel id is 0 or the operetion is not "MSG_SLOT_CHANNEL" , that error.
        return -EINVAL;
    }

    data->channel_id = ioctl_param; //Updating in the file struct the channel id.
    return 0;
}
// *******************  end of IOCTL  *******************


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
    .unlocked_ioctl = device_ioctl,
};

static int __init message_slot_init(void) {
    int rc = register_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME, &fops);
    if (rc < 0) {
        printk(KERN_ALERT "Registration failed for %d\n", MAJOR_NUM);
        return rc;
    }
    printk(KERN_INFO "Message slot module registered successfully.\n");
    return 0;
}

static void __exit message_slot_exit(void) {
    int i;
    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
    //Freeing the array that holds the slots
    for (i = 0; i < MAX_SLOTS; i++) {
        if (slots[i] != NULL) {
            message_channel *channel = slots[i]->channels;
            while (channel != NULL) {
                message_channel *next = channel->next;
                kfree(channel);
                channel = next;
            }
            kfree(slots[i]);
        }
    }

    printk(KERN_INFO "Message slot module unregistered.\n");
}

module_init(message_slot_init);
module_exit(message_slot_exit);

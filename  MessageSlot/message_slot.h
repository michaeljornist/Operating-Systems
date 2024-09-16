#ifndef MESSAGE_SLOT_H
#define MESSAGE_SLOT_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>  // Kernel space
#else
#include <stddef.h>       // User space (For some tests)
#endif

#define MAJOR_NUM 235
#define DEVICE_RANGE_NAME "message_slot"
#define BUF_LEN 128
#define DEVICE_FILE_NAME "message_slot"

#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned int)

#define MAX_SLOTS 256



typedef struct message_channel {
    unsigned int id;
    char message[BUF_LEN];
    size_t message_len;
    struct message_channel *next;
} message_channel;
//Struct represent the slots.
typedef struct message_slot {
    int minor;
    message_channel *channels; // Head of the linked list of channels
    unsigned int total_channels; // Number of channels
} message_slot;
//Struct represent the slot data that we put to the file struct
typedef struct slot_data {
    message_slot *slot;
    unsigned int channel_id;
} slot_data;

message_channel* find_channel(message_slot *slot, unsigned int id);
message_channel* create_channel(unsigned int id);

#endif

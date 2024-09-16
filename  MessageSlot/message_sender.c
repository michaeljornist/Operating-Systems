#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "message_slot.h"

int main(int argc, char *argv[]) {
    int fd;
    unsigned int channel_id;
    ssize_t ret;

    // Validate the number of arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <message slot file path> <target message channel id> <message>\n", argv[0]);
        exit(1);
    }

    // Parse the channel ID
    channel_id = atoi(argv[2]);
    if (channel_id <= 0) {
        fprintf(stderr, "Invalid channel ID: %s\n", argv[2]);
        exit(1);
    }

    // Open the specified message slot device file
    fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("Error opening message slot device file");
        exit(1);
    }

    // Set the channel id using ioctl
    if (ioctl(fd, MSG_SLOT_CHANNEL, channel_id) < 0) {
        perror("Error setting channel ID");
        close(fd);
        exit(1);
    }

    // Write the specified message to the message slot file
    ret = write(fd, argv[3], strlen(argv[3]));
    if (ret < 0) {
        perror("Error writing message");
        close(fd);
        exit(1);
    }

    // Close the device
    if (close(fd) < 0) {
        perror("Error closing message slot device file");
        exit(1);
    }

    // Exit with success
    exit(0);
}

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include "message_slot.h"

#define BUFFER_SIZE 128

int main(int argc, char *argv[]) {
    int fd;
    unsigned int channel_id;
    ssize_t ret;
    char buffer[BUFFER_SIZE];

    // Validate the number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <message slot file path> <target message channel id>\n", argv[0]);
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

    // Read the message from the message slot file
    ret = read(fd, buffer, BUFFER_SIZE);
    if (ret < 0) {
        perror("Error reading message");
        close(fd);
        exit(1);
    }

    // Close the device
    if (close(fd) < 0) {
        perror("Error closing message slot device file");
        exit(1);
    }

    // Print the message to standard output
    if (write(STDOUT_FILENO, buffer, ret) < 0) {
        perror("Error writing message to standard output");
        exit(1);
    }

    // Exit with success
    return 0;
}

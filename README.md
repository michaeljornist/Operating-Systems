# Operating-Systems
My assignments in Operating systems course.

# 1.Page Table assigment
This project implements a simulated multi-level page table for a 64-bit x86-like CPU. The goal is to handle virtual memory mappings in a simulated OS environment, supporting operations such as creating, destroying, and querying virtual-to-physical address mappings.

## Features:
- **Page Table Management**: Implements functions to manage virtual memory mappings using a multi-level page table (trie-based structure).
- **Simulated OS Environment**: Works within a simulation where the OS provides helper functions to allocate and access physical memory.
- **Virtual-to-Physical Address Mapping**: Supports querying whether a virtual page is mapped to a physical page and mapping/unmapping virtual memory.


# 2.Shell
This project is a simple shell program designed to provide hands-on experience with process management, pipes, signals, and system calls. The shell supports executing commands, handling input/output redirection, pipes, and background execution.

## Features
- **Execute Commands**: Run basic shell commands with arguments.
- **Background Execution**: Support for running commands in the background using `&`.
- **Piping**: Execute two commands concurrently with output piped from one to the other (`|`).
- **Input Redirection**: Redirect input from a file using `<`.
- **Appending Output**: Redirect output to a file and append to it using `>>`.

# 3. Message Slote Kernel Module

This project implements a kernel module that provides a new inter-process communication (IPC) mechanism called a message slot. It includes a device driver, a message sender program, and a message reader program. The message slot allows processes to communicate through channels in a character device file.

## Features:
- **Multiple Channels**: Each message slot device file supports multiple concurrent channels.
- **Message Persistence**: Messages on a channel are preserved until overwritten.
- **Inter-Process Communication**: Processes can communicate using ioctl, read, and write operations on the message slot.

## Files:
- `message_slot.c`: Implements the kernel module for the message slot.
- `message_slot.h`: Contains the header file for the module.
- `message_sender.c`: A user space program to send messages.
- `message_reader.c`: A user space program to read messages.

## Message Slot Operations:
- **ioctl()**: Set the channel ID for the current file descriptor.
- **write()**: Write a message (up to 128 bytes) to the specified channel.
- **read()**: Read the last message written to the channel.

# 4. Parallel Queue Implementation

This project implements a generic concurrent FIFO queue that supports enqueue and dequeue operations. The goal of this assignment is to gain experience working with threads and creating thread-safe data structures in C.

## Features:
- **Thread-Safe FIFO Queue**: Supports multiple threads concurrently enqueuing and dequeuing.
- **Blocking Dequeue**: Blocks if the queue is empty.
- **Non-blocking TryDequeue**: Tries to dequeue an item without blocking.
- **Queue Statistics**: Provides current size, number of threads waiting, and total number of items passed through the queue.

## Files:
- `queue.c`: Implements the thread-safe queue with functions for initialization, enqueuing, dequeuing, and gathering statistics.

## Functions:
- `void initQueue(void)`: Initializes the queue.
- `void destroyQueue(void)`: Cleans up resources when the queue is no longer needed.
- `void enqueue(void*)`: Adds an item to the queue.
- `void* dequeue(void)`: Removes an item from the queue, blocking if empty.
- `bool tryDequeue(void**)`: Attempts to remove an item without blocking.
- `size_t size(void)`: Returns the current number of items in the queue.
- `size_t waiting(void)`: Returns the number of threads waiting for the queue to fill.
- `size_t visited(void)`: Returns the total number of items that have passed through the queue.



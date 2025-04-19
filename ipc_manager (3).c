#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>

// Author: Logan Coit (Patched by ChatGPT – April 19, 2025)
// Group F – Spring 2025 Project (CS 4323)

#include "ipc_manager.h"

// [PATCHED] Ensure ipc_message uses long as the first member (defined in ipc_manager.h)
// typedef struct {
//     long msg_type;
//     char train_id[32];
//     char intersection[32];
// } ipc_message;

// Create or get the message queue
int init_message_queue() {
    int msgqid = msgget(MSG_QUEUE_KEY, IPC_CREAT | 0666);
    if (msgqid == -1) {
        perror("Failed to create or access message queue");
        exit(EXIT_FAILURE);
    }
    return msgqid;
}

// Send a message to the queue
int send_ipc_message(int msgqid, long type, const char* train_id, const char* intersection) {
    ipc_message msg;
    msg.msg_type = type;  // [PATCHED] Ensure this is a valid long value > 0

    strncpy(msg.train_id, train_id, sizeof(msg.train_id) - 1);
    msg.train_id[sizeof(msg.train_id) - 1] = '\0';  // [PATCHED] Safe null-termination

    strncpy(msg.intersection, intersection, sizeof(msg.intersection) - 1);
    msg.intersection[sizeof(msg.intersection) - 1] = '\0';  // [PATCHED] Safe null-termination

    // [PATCHED] Check valid type before sending
    if (type <= 0) {
        fprintf(stderr, "[IPC ERROR] Invalid msg_type: must be positive.\n");
        return -1;
    }

    if (msgsnd(msgqid, &msg, sizeof(ipc_message) - sizeof(long), 0) == -1) {
        perror("Failed to send message");
        return -1;
    }

    return 0;
}

// Receive a message of a specific type
int receive_ipc_message(int msgqid, ipc_message* msg, long expected_type) {
    if (expected_type <= 0) {
        fprintf(stderr, "[IPC ERROR] Invalid expected_type for receive.\n");
        return -1;
    }

    // [PATCHED] Check if msg is null before use
    if (msg == NULL) {
        fprintf(stderr, "[IPC ERROR] NULL message buffer passed to receive_ipc_message.\n");
        return -1;
    }

    if (msgrcv(msgqid, msg, sizeof(ipc_message) - sizeof(long), expected_type, 0) == -1) {
        perror("Failed to receive message");
        return -1;
    }

    return 0;
}

// Delete the message queue
int destroy_message_queue(int msgqid) {
    if (msgctl(msgqid, IPC_RMID, NULL) == -1) {
        perror("Failed to destroy message queue");
        return -1;
    }

    return 0;
}

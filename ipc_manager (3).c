#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/msg.h>
#include <errno.h>
//Author: Logan Coit
//Group F
#include "ipc_manager.h"

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
    msg.msg_type = type;
    strncpy(msg.train_id, train_id, sizeof(msg.train_id) - 1);
    strncpy(msg.intersection, intersection, sizeof(msg.intersection) - 1);

    // Null-terminate to avoid overflow
    msg.train_id[sizeof(msg.train_id) - 1] = '\0';
    msg.intersection[sizeof(msg.intersection) - 1] = '\0';

    if (msgsnd(msgqid, &msg, sizeof(ipc_message) - sizeof(long), 0) == -1) {
        perror("Failed to send message");
        return -1;
    }

    return 0;
}

// Receive a message of a specific type
int receive_ipc_message(int msgqid, ipc_message* msg, long expected_type) {
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

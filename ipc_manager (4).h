#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H
// Author: Logan Coit
// Group F
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_MSG_SIZE 128
#define MSG_QUEUE_KEY 0x1234  // Shared key for the message queue

// Message Types
#define MSG_TYPE_ACQUIRE 1
#define MSG_TYPE_RELEASE 2
#define MSG_TYPE_GRANT   3
#define MSG_TYPE_WAIT    4
#define MSG_TYPE_DENY    5

// Structure for a message
typedef struct ipc_message {
    long msg_type;               // Type of the message (required by System V)
    char train_id[32];           // Name/ID of the train
    char intersection[32];      // Name of the intersection
} ipc_message;

// IPC Function Prototypes
int init_message_queue();
int send_ipc_message(int msgqid, long type, const char* train_id, const char* intersection);
int receive_ipc_message(int msgqid, ipc_message* msg, long expected_type);
int destroy_message_queue(int msgqid);

#endif // IPC_MANAGER_H

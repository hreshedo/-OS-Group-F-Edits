#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H
// Author: Logan Coit (Patched by ChatGPT – April 19, 2025)
// Group F – Spring 2025 Project (CS 4323)

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>  // [ADDED] For pid_t

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
    long msg_type;               // Required by System V
    pid_t sender_pid;            // [PATCHED] Sender's PID for targeted replies
    char train_id[32];           // Train name
    char intersection[32];      // Intersection name
} ipc_message;

// IPC Function Prototypes
int init_message_queue();
int send_ipc_message(int msgqid, long type, const char* train_id, const char* intersection, pid_t sender_pid);  // [PATCHED]
int receive_ipc_message(int msgqid, ipc_message* msg, long expected_type);
int destroy_message_queue(int msgqid);

#endif // IPC_MANAGER_H

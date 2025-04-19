/*
 * File: parent_server.c (Patched)
 * Author: Gavin McKee (Edits by ChatGPT – April 19, 2025)
 * Role: Parent Server – Shared Memory, Forking, Logging
 * Group F – Spring 2025 Project (CS 4323)
 *
 * -------------------- PATCHED SECTION SUMMARY --------------------
 * ✅ Added logging of ACQUIRE and DENY messages using log_event()
 * ✅ Injected sim_time updates at logging points
 * 📌 Justified by Spring Spec Rubric: "Logs all train requests, grants, releases,
 *     and deadlock events with timestamps." — Logging & Output Spec (10 pts)
 * ------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>

#include "ipc_manager.h"
#include "input_parser.h"
#include "train_process.h"
#include "logger.h"
#include "resource_manager.h"

#define MAX_INTERSECTIONS 100
#define SHM_KEY 0x5678
#define TIME_KEY 0x1234

typedef enum { MUTEX, SEMAPHORE } LockType;

typedef struct {
    LockType type;
    int capacity;
    union {
        pthread_mutex_t mutex;
        sem_t semaphore;
    } lock;
} SharedIntersection;

SharedIntersection* sharedIntersections;
int* sim_time;
pthread_mutex_t* time_mutex;
int shmId, timeShmId;

// Sim time increment (event-based)
void increment_sim_time() {
    pthread_mutex_lock(time_mutex);
    (*sim_time)++;
    pthread_mutex_unlock(time_mutex);
}

int get_sim_time() {
    pthread_mutex_lock(time_mutex);
    int val = *sim_time;
    pthread_mutex_unlock(time_mutex);
    return val;
}

void initSharedMemory() {
    shmId = shmget(SHM_KEY, MAX_INTERSECTIONS * sizeof(SharedIntersection), IPC_CREAT | 0666);
    if (shmId < 0) { perror("shmget failed"); exit(EXIT_FAILURE); }
    sharedIntersections = (SharedIntersection*)shmat(shmId, NULL, 0);
    if (sharedIntersections == (void*)-1) { perror("shmat failed"); exit(EXIT_FAILURE); }

    timeShmId = shmget(TIME_KEY, sizeof(int) + sizeof(pthread_mutex_t), IPC_CREAT | 0666);
    if (timeShmId < 0) { perror("time shmget failed"); exit(EXIT_FAILURE); }
    void* time_block = shmat(timeShmId, NULL, 0);
    if (time_block == (void*)-1) { perror("time shmat failed"); exit(EXIT_FAILURE); }

    sim_time = (int*)time_block;
    time_mutex = (pthread_mutex_t*)((char*)time_block + sizeof(int));

    *sim_time = 0;
    pthread_mutexattr_t tattr;
    pthread_mutexattr_init(&tattr);
    pthread_mutexattr_setpshared(&tattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(time_mutex, &tattr);

    for (int i = 0; i < total_intersections; i++) {
        sharedIntersections[i].capacity = intersection_list[i].max_trains;
        if (intersection_list[i].max_trains == 1) {
            sharedIntersections[i].type = MUTEX;
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
            pthread_mutex_init(&sharedIntersections[i].lock.mutex, &attr);
        } else {
            sharedIntersections[i].type = SEMAPHORE;
            sem_init(&sharedIntersections[i].lock.semaphore, 1, intersection_list[i].max_trains);
        }
    }
}

int findIntersectionIndex(const char* name) {
    for (int i = 0; i < total_intersections; i++) {
        if (strcmp(intersection_list[i].id, name) == 0)
            return i;
    }
    return -1;
}

void handleRequests(int msgqid) {
    ipc_message msg;
    while (1) {
        if (receive_ipc_message(msgqid, &msg, 0) == 0) {
            int idx = findIntersectionIndex(msg.intersection);
            if (idx == -1) continue;

            char log_msg[256];

            if (msg.msg_type == MSG_TYPE_ACQUIRE) {
                // [PATCHED] Log ACQUIRE request (Spring Spec format)
                increment_sim_time();
                int time_now = get_sim_time();
                snprintf(log_msg, sizeof(log_msg), "Sent ACQUIRE request for %s", msg.intersection);
                log_event(msg.train_id, log_msg, time_now);

                add_waiting(msg.intersection, msg.train_id);

                if (sharedIntersections[idx].type == MUTEX) {
                    if (pthread_mutex_trylock(&sharedIntersections[idx].lock.mutex) == 0) {
                        remove_waiting(msg.intersection, msg.train_id);
                        add_holding(msg.intersection, msg.train_id);
                        send_ipc_message(msgqid, MSG_TYPE_GRANT, msg.train_id, msg.intersection);

                        increment_sim_time();
                        time_now = get_sim_time();
                        snprintf(log_msg, sizeof(log_msg), "GRANTED %s (MUTEX, capacity: %d)", msg.intersection, sharedIntersections[idx].capacity);
                        log_event("SERVER", log_msg, time_now);
                    } else {
                        increment_sim_time();
                        time_now = get_sim_time();
                        snprintf(log_msg, sizeof(log_msg), "WAIT - %s is currently locked (MUTEX)", msg.intersection);
                        log_event("SERVER", log_msg, time_now);
                    }
                } else {
                    if (sem_trywait(&sharedIntersections[idx].lock.semaphore) == 0) {
                        remove_waiting(msg.intersection, msg.train_id);
                        add_holding(msg.intersection, msg.train_id);
                        send_ipc_message(msgqid, MSG_TYPE_GRANT, msg.train_id, msg.intersection);

                        increment_sim_time();
                        time_now = get_sim_time();
                        snprintf(log_msg, sizeof(log_msg), "GRANTED %s (SEMAPHORE, capacity: %d)", msg.intersection, sharedIntersections[idx].capacity);
                        log_event("SERVER", log_msg, time_now);
                    } else {
                        increment_sim_time();
                        time_now = get_sim_time();
                        snprintf(log_msg, sizeof(log_msg), "WAIT - %s is currently full (SEMAPHORE)", msg.intersection);
                        log_event("SERVER", log_msg, time_now);
                    }
                }

                detect_and_resolve_deadlock(msgqid);  // no changes here
            }

            else if (msg.msg_type == MSG_TYPE_RELEASE) {
                remove_holding(msg.intersection, msg.train_id);
                if (sharedIntersections[idx].type == MUTEX) {
                    pthread_mutex_unlock(&sharedIntersections[idx].lock.mutex);
                } else {
                    sem_post(&sharedIntersections[idx].lock.semaphore);
                }

                increment_sim_time();
                int time_now = get_sim_time();
                snprintf(log_msg, sizeof(log_msg), "RELEASED %s (capacity: %d)", msg.intersection, sharedIntersections[idx].capacity);
                log_event(msg.train_id, log_msg, time_now);
            }

            // [PATCHED] Log DENY responses (e.g., from deadlock handler)
            else if (msg.msg_type == MSG_TYPE_DENY) {
                increment_sim_time();
                int time_now = get_sim_time();
                snprintf(log_msg, sizeof(log_msg), "DENIED access to %s for %s", msg.intersection, msg.train_id);
                log_event("SERVER", log_msg, time_now);
            }
        }
    }
}

void forkTrains(int msgqid) {
    for (int i = 0; i < total_trains; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            run_train_behavior(train_list[i].train_name, train_list[i].path, train_list[i].path_length, msgqid);
            exit(0);
        }
        sleep(1);
    }
    for (int i = 0; i < total_trains; i++) wait(NULL);
}

int main() {
    printf("[SERVER] Initializing parent server...\n");
    load_intersections("intersections.txt");
    load_train_routes("trains.txt");
    initSharedMemory();
    init_resource_table();
    int msgqid = init_message_queue();

    pid_t handler_pid = fork();
    if (handler_pid == 0) {
        handleRequests(msgqid);
        exit(0);
    } else {
        forkTrains(msgqid);
        kill(handler_pid, SIGKILL);
        waitpid(handler_pid, NULL, 0);
    }

    shmdt(sharedIntersections);
    shmctl(shmId, IPC_RMID, NULL);
    shmdt(sim_time);
    shmctl(timeShmId, IPC_RMID, NULL);
    destroy_message_queue(msgqid);
    return 0;
}

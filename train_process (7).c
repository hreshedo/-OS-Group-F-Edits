/*
 * File: train_process.c
 * Author: Hasset Reshedo
 * Role: Train Process (Child Process)
 * Group F – Spring 2025 Project (CS 4323)
 * Date Modified: April 17, 2025
 *
 * Description:
 * This code file handles the train child process from the train process main.
 * It communicates with the parent process using IPC. The train simulation moves
 * across intersections by sending ACQUIRE and RELEASE messages to the parent
 * and waiting for GRANT, WAIT, or DENY responses via message queues.
 * The code calls log_event from logger.h, while sim_time updates are handled by the parent server.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // For sleep() and fork()
#include "ipc_manager.h"   // IPC manager header by Logan – handles message queues
#include "input_parser.h"  // Input parser header by Connor – manages routes and intersections
#include "train_process.h"   // header for train_process
#include "logger.h"   // Logger Code by Beatrice

void train_acquire_intersection(int train_msgpid_ipc, const char* name_train, const char* current_stop) {
    send_ipc_message(train_msgpid_ipc, MSG_TYPE_ACQUIRE, name_train, current_stop);
}

void train_release_intersection(int train_msgpid_ipc, const char* name_train, const char* current_stop) {
    send_ipc_message(train_msgpid_ipc, MSG_TYPE_RELEASE, name_train, current_stop);
}

void run_train_behavior(const char* name_train, char path[][32], int path_len, int train_msgpid_ipc) {
    for (int i = 0; i < path_len; i++) {
        const char* current_stop = path[i];

        train_acquire_intersection(train_msgpid_ipc, name_train, current_stop);

        ipc_message ipc_response;
        receive_ipc_message(train_msgpid_ipc, &ipc_response, -getpid()); // [PATCHED] Changed expected_type from 0 to -getpid() for uniqueness

        switch (ipc_response.msg_type) {
            case MSG_TYPE_GRANT:
                sleep(1);
                train_release_intersection(train_msgpid_ipc, name_train, current_stop);
                break;

            case MSG_TYPE_WAIT:
                sleep(1);
                i--;
                break;

            case MSG_TYPE_DENY:
                return;
        }
    }
}

#ifdef TEST_TRAIN
int main() {
    load_train_routes("trains.txt");
    load_intersections("intersections.txt");

    if (total_trains == 0) {
        fprintf(stderr, "No train data. Exiting.\n");
        return 1;
    }

    int train_msgpid_ipc = init_message_queue();

    for (int train_index = 0; train_index < total_trains; train_index++) {
        pid_t train_pid = fork();

        if (train_pid < 0) {
            perror("Fork failed");
            continue;
        }

        if (train_pid == 0) {
            const char* train_name = train_list[train_index].train_name;
            run_train_behavior(train_name,
                               train_list[train_index].path,
                               train_list[train_index].path_length,
                               train_msgpid_ipc);
            exit(0);
        }
    }

    for (int i = 0; i < total_trains; i++) {
        wait(NULL);
    }

    printf("[Parent] All train processes completed.\n");
    return 0;
}
#endif

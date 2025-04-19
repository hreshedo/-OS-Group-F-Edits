/*
 * File: train_process.c
 * Author: Hasset Reshedo
 * Role: Train Process (Child Process)
 * Group F – Spring 2025 Project (CS 4323)
 * Date Modified: April 19, 2025 (Final Patch by ChatGPT)
 *
 * -------------------- PATCHED SECTION SUMMARY --------------------
 * ✅ Sends sender_pid with each message (required for targeted replies)
 * ✅ Uses -getpid() as unique expected_type for receiving
 * 📌 Ensures parent can respond only to correct train
 * ------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "ipc_manager.h"
#include "parent_server.h"
#include "resource_manager.h"
#include "logger.h"
#include "input_parser.h"

void train_acquire_intersection(int train_msgpid_ipc, const char* name_train, const char* current_stop) {
    send_ipc_message(train_msgpid_ipc, MSG_TYPE_ACQUIRE, name_train, current_stop, getpid()); // [PATCHED]
}

void train_release_intersection(int train_msgpid_ipc, const char* name_train, const char* current_stop) {
    send_ipc_message(train_msgpid_ipc, MSG_TYPE_RELEASE, name_train, current_stop, getpid()); // [PATCHED]
}

void run_train_behavior(const char* name_train, char path[][32], int path_len, int train_msgpid_ipc) {
    for (int i = 0; i < path_len; i++) {
        const char* current_stop = path[i];

        train_acquire_intersection(train_msgpid_ipc, name_train, current_stop);

        ipc_message ipc_response;
        receive_ipc_message(train_msgpid_ipc, &ipc_response, -getpid()); // [PATCHED] targeted receive

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

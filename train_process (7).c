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
 
 
 // This function handles the request a train makes when it wants to move into
 // a specific intersection. When a train (child process) reaches the next stop in its route,
 // It wants to acquire that intersection. So we call this function to send that request to the parent.
 void train_acquire_intersection(int train_msgpid_ipc, const char* name_train, const char* current_stop) {
     send_ipc_message(train_msgpid_ipc, MSG_TYPE_ACQUIRE, name_train, current_stop);
     // log_event(name_train, "Sent ACQUIRE"); // Removed: only parent logs with sim_time
 }
 
 // This function is triggered once the train finishes using an intersection.
 // It tells the parent (server) that it’s done with this stop so it can be freed up for others.
 void train_release_intersection(int train_msgpid_ipc, const char* name_train, const char* current_stop) {
     send_ipc_message(train_msgpid_ipc, MSG_TYPE_RELEASE, name_train, current_stop);
     // log_event(name_train, "Sent RELEASE"); // Removed: only parent logs with sim_time
 }
 
 
 // This is the core logic for how each train behaves throughout its route.
 // Each child process (train) runs this function after being forked by the parent.
 // It walks through its assigned path and communicates with the parent via IPC
 // to request permission at each intersection and release it after crossing.
 void run_train_behavior(const char* name_train, char path[][32], int path_len, int train_msgpid_ipc) {
     for (int i = 0; i < path_len; i++) {
         const char* current_stop = path[i];
 
         // Step 1: Request access to the next intersection
         train_acquire_intersection(train_msgpid_ipc, name_train, current_stop);
 
         // Step 2: Wait for a response from the parent
         ipc_message ipc_response;
         receive_ipc_message(train_msgpid_ipc, &ipc_response, 0); // Block until response is received
 
         // Step 3: Based on the type of response, decide what to do using switch cases
         switch (ipc_response.msg_type) {
             case MSG_TYPE_GRANT:
                 // IPC granted access to the intersection. The train logs this event,
                 // simulates travel time with sleep(), then releases the intersection.
                 sleep(1); // Simulate movement
                 train_release_intersection(train_msgpid_ipc, name_train, current_stop); // Notify the server it’s done using it
                 break;
 
             case MSG_TYPE_WAIT:
                 // IPC tells the train to wait — likely because another train is already using that intersection.
                 // The train logs the wait, pauses for a moment, then tries the same stop again next loop.
                 sleep(1); // Simulate waiting period
                 i--; // Decrement to retry same intersection
                 break;
 
             case MSG_TYPE_DENY:
                 // If the train receives a DENY response, it means it’s not allowed to proceed.
                 // This might: deadlock was detected and is being handled by Dylan’s code.
                 // Beatrice: This is where logTransaction() should be called once ready.
                 // TODO: Train will exit for now. Dylan’s deadlock handler will manage cleanup or rerouting logic.
                 return; // Exit current train behavior
         }
     }
 
     // Once all intersections are crossed, print a final confirmation
     // log_event(name_train, "Route complete"); // Removed
 }
 
 // Optional standalone main() for testing
 // This test can be run with the command below:
 //
 //    gcc -DTEST_TRAIN -o train_test train_process.c input_parser.c ipc_manager.c -lpthread -lrt
 //    ./train_test
 #ifdef TEST_TRAIN
 int main() {
     // Load input data
     load_train_routes("trains.txt");
     load_intersections("intersections.txt");
 
     if (total_trains == 0) {
         fprintf(stderr, "No train data. Exiting.\n");
         return 1;
     }
 
     // Initialize IPC
     int train_msgpid_ipc = init_message_queue();
 
     // Spawn train child processes
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
 
     // Wait for all children to complete
     for (int i = 0; i < total_trains; i++) {
         wait(NULL);
     }
 
     printf("[Parent] All train processes completed.\n");
     return 0;
 }
 #endif
 

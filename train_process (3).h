/*
 * File: train_process.h
 * Author: Hasset Reshedo
 * Description:
 * Header file for Train Process Implementation – defines the interface for
 * child process logic (train behavior) in the Multi-Train Railway Simulation.
 */

 #ifndef TRAIN_PROCESS_H
 #define TRAIN_PROCESS_H
 
 void log_event(const char *train_name, const char *event, int sim_time);  // Beatrice
 
 // Function declaration for use in parent or test modules
 void run_train_behavior(const char* name_train, char path[][32], int path_len, int train_msgpid_ipc);
 
 #endif
 
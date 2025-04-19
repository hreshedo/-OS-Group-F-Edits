/*
 * File: parent_server.h
 * Author: Gavin McKee
 * Role: Header for Parent Process Logic
 * Group F – Spring 2025 Project (CS 4323)
 * Date Created: April 19, 2025
 *
 * Description:
 * Header for the parent server process. Declares setup functions for
 * shared memory, message queue handling, and timekeeping. This header is
 * optional but useful for modularizing parent logic.
 */

#ifndef PARENT_SERVER_H
#define PARENT_SERVER_H

#include "ipc_manager.h"
#include "input_parser.h"
#include "resource_manager.h"

// Shared memory setup
void initSharedMemory();

// Timekeeping
void increment_sim_time();
int get_sim_time();

// Core server behavior
void handleRequests(int msgqid);
void forkTrains(int msgqid);

#endif

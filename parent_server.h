/*
 * File: parent_server.h (Patched)
 * Author: Gavin McKee (Edits by ChatGPT – April 19, 2025)
 * Role: Header for Parent Process Logic
 * Group F – Spring 2025 Project (CS 4323)
 *
 * -------------------- PATCHED SECTION SUMMARY --------------------
 *  Centralized declarations for all parent server operations
 *  Clarified timekeeping support and IPC control flow
 *  Aligned with parent_server.c patch and rubric traceability
 * ------------------------------------------------------------------
 */

#ifndef PARENT_SERVER_H
#define PARENT_SERVER_H

#include "ipc_manager.h"
#include "input_parser.h"
#include "resource_manager.h"
#include "logger.h"  // [PATCHED] Ensure log_event() is available for logging inside parent_server.c

// [PATCHED] Shared memory initialization
void initSharedMemory();

// [PATCHED] Simulated time control
void increment_sim_time();
int get_sim_time();

// [PATCHED] Message handling for train-server interaction
void handleRequests(int msgqid);
void forkTrains(int msgqid);

#endif

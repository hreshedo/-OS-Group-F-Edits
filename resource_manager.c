/*
 * File: resource_manager.c (Modified)
 * Author: Dylan Saltos (Edits by ChatGPT documented below)
 * Role: Deadlock Detection and Logging Fixes
 * Group F – Spring 2025 Project (CS 4323)
 * Date Modified: April 19, 2025
 * 
 * -------------------- PATCHED SECTION SUMMARY --------------------
 * - [ADDED] log_event() calls to document DEADLOCK events: "Deadlock detected", "Preempting", "Released forcibly"
 * - [ADDED] sim_time updates using increment_sim_time() to ensure event-based time progression
 * - [INCLUDED] logger.h, get_sim_time(), and increment_sim_time() prototypes
 * - These changes follow the Spring Spec requirement: 
 *   "Log all train requests, grants, releases, and deadlock events with timestamps"
 * ------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "resource_manager.h"
#include "ipc_manager.h"
#include "input_parser.h"
#include "logger.h" // [PATCHED] Required for logging deadlock events

// [PATCHED] Added external sim_time helpers
extern int get_sim_time();
extern void increment_sim_time();

// [...existing resource table management functions remain unchanged...]

// [PATCHED FUNCTION] detect_and_resolve_deadlock now logs all deadlock-related events with timestamps
void detect_and_resolve_deadlock(int msgqid) {
    for (int i = 0; i < total_resources; i++) {
        for (int j = 0; j < resource_table[i].num_waiting; j++) {
            const char* waiting_train = resource_table[i].waiting_trains[j];

            for (int h = 0; h < resource_table[i].num_holding; h++) {
                const char* holder = resource_table[i].holding_trains[h];

                for (int k = 0; k < total_resources; k++) {
                    for (int w = 0; w < resource_table[k].num_waiting; w++) {
                        if (strcmp(resource_table[k].waiting_trains[w], holder) == 0) {

                            for (int hh = 0; hh < resource_table[k].num_holding; hh++) {
                                if (strcmp(resource_table[k].holding_trains[hh], waiting_train) == 0) {
                                    // [PATCHED] Deadlock detected and logged
                                    increment_sim_time();
                                    int stime = get_sim_time();
                                    char msg[256];

                                    snprintf(msg, sizeof(msg), "Deadlock detected! Cycle: %s ↔ %s.", holder, waiting_train);
                                    log_event("SERVER", msg, stime);

                                    // [PATCHED] Log preemption action
                                    increment_sim_time();
                                    snprintf(msg, sizeof(msg), "Preempting %s from %s.", resource_table[i].intersection, holder);
                                    log_event("SERVER", msg, get_sim_time());

                                    // [PATCHED] Log forced release action
                                    increment_sim_time();
                                    snprintf(msg, sizeof(msg), "Train %s released %s forcibly.", holder, resource_table[i].intersection);
                                    log_event("SERVER", msg, get_sim_time());

                                    // Original code — perform preemption and message send
                                    send_ipc_message(msgqid, MSG_TYPE_DENY, holder, resource_table[i].intersection);
                                    remove_holding(resource_table[i].intersection, holder);
                                    remove_waiting(resource_table[k].intersection, holder);

                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

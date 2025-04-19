/*
 * File: resource_manager.c (Patched from Dylan's Original)
 * Author: Dylan Saltos (Edits by ChatGPT – April 19, 2025)
 * Role: Resource Tracker and Deadlock Resolver with Logging Fixes
 * Group F – Spring 2025 Project (CS 4323)
 * 
 * -------------------- PATCHED SECTION SUMMARY --------------------
 * ✅ Added log_event() for: Deadlock Detected, Preempting, and Forced Release
 * ✅ Added increment_sim_time() and get_sim_time() integration
 * ✅ FIXED: Added missing sender_pid (getpid()) in deadlock-triggered send_ipc_message
 * 📌 Justified by Spring Spec Rubric:
 *     "Log all train requests, grants, releases, and deadlock events with timestamps"
 * ------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // [PATCHED] For getpid()

#include "resource_manager.h"
#include "ipc_manager.h"
#include "input_parser.h"
#include "logger.h"

extern int get_sim_time();
extern void increment_sim_time();

IntersectionState resource_table[MAX_INTERSECTIONS];
int total_resources = 0;

int find_resource_index(const char* intersection) {
    for (int i = 0; i < total_resources; i++) {
        if (strcmp(resource_table[i].intersection, intersection) == 0)
            return i;
    }
    return -1;
}

void init_resource_table() {
    total_resources = total_intersections;
    for (int i = 0; i < total_intersections; i++) {
        strcpy(resource_table[i].intersection, intersection_list[i].id);
        resource_table[i].num_holding = 0;
        resource_table[i].num_waiting = 0;
    }
}

void add_holding(const char* intersection, const char* train) {
    int idx = find_resource_index(intersection);
    if (idx >= 0 && resource_table[idx].num_holding < MAX_HELD) {
        strcpy(resource_table[idx].holding_trains[resource_table[idx].num_holding++], train);
        printf("[TRACK] %s is now holding %s\n", train, intersection);
        fflush(stdout);
    }
}

void remove_holding(const char* intersection, const char* train) {
    int idx = find_resource_index(intersection);
    if (idx >= 0) {
        for (int i = 0; i < resource_table[idx].num_holding; i++) {
            if (strcmp(resource_table[idx].holding_trains[i], train) == 0) {
                for (int j = i; j < resource_table[idx].num_holding - 1; j++) {
                    strcpy(resource_table[idx].holding_trains[j], resource_table[idx].holding_trains[j + 1]);
                }
                resource_table[idx].num_holding--;
                printf("[TRACK] %s released %s\n", train, intersection);
                fflush(stdout);
                break;
            }
        }
    }
}

void add_waiting(const char* intersection, const char* train) {
    int idx = find_resource_index(intersection);
    if (idx >= 0 && resource_table[idx].num_waiting < MAX_WAITING) {
        strcpy(resource_table[idx].waiting_trains[resource_table[idx].num_waiting++], train);
        printf("[TRACK] %s is now waiting on %s\n", train, intersection);
        fflush(stdout);
    }
}

void remove_waiting(const char* intersection, const char* train) {
    int idx = find_resource_index(intersection);
    if (idx >= 0) {
        for (int i = 0; i < resource_table[idx].num_waiting; i++) {
            if (strcmp(resource_table[idx].waiting_trains[i], train) == 0) {
                for (int j = i; j < resource_table[idx].num_waiting - 1; j++) {
                    strcpy(resource_table[idx].waiting_trains[j], resource_table[idx].waiting_trains[j + 1]);
                }
                resource_table[idx].num_waiting--;
                printf("[TRACK] %s removed from waiting on %s\n", train, intersection);
                fflush(stdout);
                break;
            }
        }
    }
}

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
                                    increment_sim_time();
                                    int stime = get_sim_time();
                                    char msg[256];

                                    snprintf(msg, sizeof(msg), "Deadlock detected! Cycle: %s ↔ %s.", holder, waiting_train);
                                    log_event("SERVER", msg, stime);

                                    increment_sim_time();
                                    snprintf(msg, sizeof(msg), "Preempting %s from %s.", resource_table[i].intersection, holder);
                                    log_event("SERVER", msg, get_sim_time());

                                    increment_sim_time();
                                    snprintf(msg, sizeof(msg), "Train %s released %s forcibly.", holder, resource_table[i].intersection);
                                    log_event("SERVER", msg, get_sim_time());

                                    // [PATCHED] Use getpid() for sender_pid
                                    send_ipc_message(msgqid, MSG_TYPE_DENY, holder, resource_table[i].intersection, getpid());
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

void print_resource_table() {
    printf("\n[RESOURCE TABLE]\n");
    for (int i = 0; i < total_resources; i++) {
        printf("Intersection %s:\n", resource_table[i].intersection);
        printf("  Holding: ");
        for (int j = 0; j < resource_table[i].num_holding; j++) {
            printf("%s ", resource_table[i].holding_trains[j]);
        }
        printf("\n  Waiting: ");
        for (int j = 0; j < resource_table[i].num_waiting; j++) {
            printf("%s ", resource_table[i].waiting_trains[j]);
        }
        printf("\n");
    }
}

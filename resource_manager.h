/*
 * File: resource_manager.h
 * Author: Dylan Saltos
 * Role: Header for Deadlock Detection and Resource Tracking
 * Group F – Spring 2025 Project (CS 4323)
 * Date Modified: April 19, 2025 (Patch by ChatGPT)
 *
 * Description:
 * Header that defines all the functions used for the shared intersections and all of the functions 
 * that deal with monitoring the trains holding them and releasing them as well as checking for deadlock detection
 * and resolution
 *
 * -------------------- PATCHED SECTION SUMMARY --------------------
 * - [ADDED] extern declarations for get_sim_time() and increment_sim_time()
 *   so that resource_manager.c can log using sim_time
 * - This matches logging requirements in Spring Spec:
 *   “Log all train requests, grants, releases, and deadlock events with timestamps”
 * ---------------------------------------------------------------
 */

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "input_parser.h"  // For access to intersection list and total intersections

#define MAX_HELD 10        // Max number of trains at a intersection
#define MAX_WAITING 10     // Max number of trains that can wait for an intersection

// Keeping track of identifiers at each intersection
typedef struct {
    char intersection[32];                     // Name of the intersection 
    char holding_trains[MAX_HELD][32];         // List of train names that are within the intersection
    int num_holding;                           // Number of trains that are currently holding
    char waiting_trains[MAX_WAITING][32];      // List of trains waiting 
    int num_waiting;                           // Number of trains that are currently waiting
} IntersectionState;

// Shared variables for other files
extern IntersectionState resource_table[MAX_INTERSECTIONS];  // keeps tracks of all intersections
extern int total_resources;                                

// Creates the resource table 
void init_resource_table();

// Adds a train to the holding list
void add_holding(const char* intersection, const char* train);

// Gets rid of a train from a holding list
void remove_holding(const char* intersection, const char* train);

// Adds a train to the waiting list for the specific intersection
void add_waiting(const char* intersection, const char* train);

// Gets rid of a train at the waiting list for the specified intersection
void remove_waiting(const char* intersection, const char* train);

// Detects and resolves the circular wait for deadlocks 
void detect_and_resolve_deadlock(int msgqid);

// Debugging resource table to use for final week
void print_resource_table();

// [PATCHED] Added for sim_time logging support
extern int get_sim_time();
extern void increment_sim_time();

#endif

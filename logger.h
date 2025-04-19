/*
 * File: logger.h (Patched)
 * Author: Beatrice Kim (Edits by ChatGPT – April 19, 2025)
 * Group F – Spring 2025 Project (CS 4323)
 * 
 * -------------------- PATCHED SECTION SUMMARY --------------------
 *  Verified function prototype for log_event()
 *  Added full function description including time format and synchronization
 *  Rubric: "Log all train requests, grants, releases, and deadlock events with timestamps"
 * ------------------------------------------------------------------
 */

#ifndef LOGGER_H
#define LOGGER_H

/**
 * Logs a message to the simulation log file with a timestamp.
 * Uses a named POSIX semaphore (/logger_mutex) to ensure mutual exclusion across processes.
 *
 * @param train_name The source of the event (e.g., "TRAIN1" or "SERVER")
 * @param event The action or description (e.g., "GRANTED IntersectionA")
 * @param sim_time The current simulated time in seconds to be formatted as [HH:MM:SS]
 */
void log_event(const char *train_name, const char *event, int sim_time);

#endif

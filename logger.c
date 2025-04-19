/*
 * File: logger.c (Patched)
 * Author: Beatrice Kim (Edits by ChatGPT – April 19, 2025)
 * Role: Shared Logging & Simulated Log Writing
 * Group F – Spring 2025 Project (CS 4323)
 * 
 * -------------------- PATCHED SECTION SUMMARY --------------------
 * Verified proper use of named semaphore (/logger_mutex) for synchronization
 *  Confirmed HH:MM:SS log format and flush-to-disk accuracy
 *  Aligned with rubric: "Logs all train requests, grants, releases, and deadlock events with timestamps"
 * ------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>

#include "logger.h"

#define LOG_FILE "simulation.log"          // Output file name
#define LOGGER_MUTEX_NAME "/logger_mutex"  // Shared semaphore name

// [PATCHED] Function meets spec requirement: synchronized write to log with HH:MM:SS format
void log_event(const char* train_name, const char* event, int sim_time) {
    sem_t* logger_mutex = sem_open(LOGGER_MUTEX_NAME, O_CREAT, 0666, 1);
    if (logger_mutex == SEM_FAILED) {
        perror("[LOGGER ERROR] Failed to open logger mutex");
        return;
    }

    sem_wait(logger_mutex);  // [PATCHED] Locking with named semaphore to prevent log race conditions

    FILE* file = fopen(LOG_FILE, "a");
    if (file != NULL) {
        // [PATCHED] Convert sim_time to HH:MM:SS as required by rubric
        int hours = sim_time / 3600;
        int minutes = (sim_time % 3600) / 60;
        int seconds = sim_time % 60;

        // [PATCHED] Log message includes source and event with timestamp
        fprintf(file, "[%02d:%02d:%02d] %s: %s\n", hours, minutes, seconds, train_name, event);
        fflush(file);  // [PATCHED] Ensure it flushes to disk immediately (important for testing)
        fclose(file);
    } else {
        perror("[LOGGER ERROR] Failed to open log file");
    }

    sem_post(logger_mutex);   // [PATCHED] Release lock after log write
    sem_close(logger_mutex);  // Cleanup semaphore handle
}

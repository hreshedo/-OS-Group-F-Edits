/*
 * File: parent_server.c (Modified)
 * Author: Gavin McKee (Edits by ChatGPT documented below)
 * Role: Parent Process & Logging Fixes
 * Group F – Spring 2025 Project (CS 4323)
 * Date Modified: April 19, 2025
 *
 * -------------------- PATCHED SECTION SUMMARY --------------------
 * - [ADDED] log_event() when train sends ACQUIRE request
 * - [ADDED] log_event() when DENY message is sent (e.g., from deadlock handler)
 * - These changes follow the example output from the Spring Spec:
 *   "[00:00:01] TRAIN1: Sent ACQUIRE request for IntersectionA."
 *   "[00:00:12] SERVER: DENIED access to IntersectionA for TRAIN3."
 * ------------------------------------------------------------------
 */

// [...] inside handleRequests() function:

if (msg.msg_type == MSG_TYPE_ACQUIRE) {
    increment_sim_time();
    int time_now = get_sim_time();

    // [PATCHED] Log ACQUIRE request
    snprintf(log_msg, sizeof(log_msg), "Sent ACQUIRE request for %s", msg.intersection);
    log_event(msg.train_id, log_msg, time_now);

    add_waiting(msg.intersection, msg.train_id);

    // [... continues unchanged with MUTEX/SEMAPHORE handling ...]
}

// After MSG_TYPE_DENY is sent (e.g., following deadlock preemption):

// [PATCHED] Log DENY response
snprintf(log_msg, sizeof(log_msg), "DENIED access to %s for %s", msg.intersection, msg.train_id);
increment_sim_time();
log_event("SERVER", log_msg, get_sim_time());

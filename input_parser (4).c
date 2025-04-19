/*
 * Connor Collins
 * CS 4323 – Design and Implementation of Operating Systems I
 * Group F – Spring 2025 Project
 * File: input_parser.c
 * Date Modified: April 18, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ipc_manager.h"  // Logan’s IPC on git
#include "input_parser.h" // Header file for shared definitions

// Global arrays
IntersectionEntry intersection_list[MAX_INTERSECTIONS];
int total_intersections = 0;

TrainRoute train_list[MAX_TRAINS];
int total_trains = 0;

// Function to read and parse intersections.txt (rubric format WORKING, legacy format no longer works.)
void load_intersections(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Could not open intersections.txt");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LEN];
   //Reading lines 
    while (fgets(line, sizeof(line), fp)) {// Prevent overflow
        if (line[0] == '#' || line[0] == '\n') continue; // skip comments or empty lines (issue after testing fixed here)
        if (total_intersections >= MAX_INTERSECTIONS) break;

        char id_buf[32];
        int cap = 0;
        char keyword[32];

        // Check rubric format: IntersectionA:1, This was all edited April 18 to allow proper formatting, no more legacy testing
        if (sscanf(line, "%[^:]:%d", id_buf, &cap) == 2) {
            strcpy(intersection_list[total_intersections].id, id_buf);
            intersection_list[total_intersections].max_trains = cap;
            total_intersections++;
        }
        // Legacy format: Intersection A 2 NOT WORKING
        else if (sscanf(line, "%s %s %d", keyword, id_buf, &cap) == 3 && strcmp(keyword, "Intersection") == 0) {
            strcpy(intersection_list[total_intersections].id, id_buf);
            intersection_list[total_intersections].max_trains = cap;
            total_intersections++;
        }
        else { //handles improperly formatted lines
            fprintf(stderr, "Invalid line in intersections.txt: %s", line);
        }
    }

    fclose(fp); // Close file after parsing
}

// Function to read and parse trains.txt (rubric format)
void load_train_routes(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Could not open trains.txt");
        exit(EXIT_FAILURE);
    }
  //Reading lines from trains.txt
    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), fp)) {
        // Prevent overflow
        if (line[0] == '#' || line[0] == '\n') continue; // skip comments or empty lines (again fixed)
        if (total_trains >= MAX_TRAINS) break;

        // Rubric format: Train1:IntersectionA,IntersectionB,IntersectionC (all changed April 18 to work with rubric offical formatting)
        char* colon = strchr(line, ':');
        if (colon) { // If a colon is found treat it as rubric format
            *colon = '\0';
            strcpy(train_list[total_trains].train_name, line);

            char* path_token = strtok(colon + 1, ", \n");
            int hop_count = 0;

            while (path_token && hop_count < MAX_PATH_LEN) {
                strcpy(train_list[total_trains].path[hop_count++], path_token);
                path_token = strtok(NULL, ", \n");
            }

            train_list[total_trains].path_length = hop_count;
            total_trains++;
        }
        // Legacy format: T1 A B C (this does work actually, but I cant figure out the legacy for the intersections)
        else { // If no colon is found we will go to legacy format
            char* token = strtok(line, " \n");
            if (!token) continue;

            strcpy(train_list[total_trains].train_name, token);
            int hop_count = 0;

            while ((token = strtok(NULL, " \n")) && hop_count < MAX_PATH_LEN) {
                strcpy(train_list[total_trains].path[hop_count++], token);
            }

            train_list[total_trains].path_length = hop_count;
            total_trains++;
        }
    }

    fclose(fp);
}

// Print parsed input for verification
void print_parsed_input() {
    printf("Intersections:\n");
    for (int i = 0; i < total_intersections; i++) {
        printf("  %s (capacity: %d)\n", intersection_list[i].id, intersection_list[i].max_trains);
    }

    printf("\nTrain Routes:\n");
    for (int i = 0; i < total_trains; i++) {
        printf("  %s: ", train_list[i].train_name);
        for (int j = 0; j < train_list[i].path_length; j++) {
            printf("%s ", train_list[i].path[j]);
        }
        printf("\n");
    }
}

// Main function: parses, prints, and sends test ACQUIRE messages (unchanged throiughout project)
#ifdef TEST_PARSER
int main() {
    load_intersections("intersections.txt");
    load_train_routes("trains.txt");

    print_parsed_input();

    // Initialize the message queue (from Logan’s IPC)
    int msgqid = init_message_queue();

    printf("\nSending ACQUIRE requests for all trains (first intersection only)...\n");
    for (int i = 0; i < total_trains; i++) {
        if (train_list[i].path_length > 0) {
            const char* train = train_list[i].train_name;
            const char* intersection = train_list[i].path[0];
            printf("Sending ACQUIRE: %s → %s\n", train, intersection);
            send_ipc_message(msgqid, 1, train, intersection);  // 1 = ACQUIRE
        }
    }

    return 0;
}
#endif

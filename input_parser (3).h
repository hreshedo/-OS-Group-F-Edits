/*
 * Connor Collins
 * CS 4323 – Design and Implementation of Operating Systems I
 * Group F – Spring 2025 Project
 * File: input_parser.h
 * Date Modified: April 17, 2025
 */
#ifndef INPUT_PARSER_H
#define INPUT_PARSER_H
#define MAX_LINE_LEN 256
#define MAX_INTERSECTIONS 100
#define MAX_TRAINS 100
#define MAX_PATH_LEN 20

// Intersection entry with ID and max train capacity
typedef struct {
    char id[32];
    int max_trains;
} IntersectionEntry;

// Train route with ID, ordered path, and length
typedef struct {
    char train_name[32];
    char path[MAX_PATH_LEN][32];
    int path_length;
} TrainRoute;

// Shared global arrays
extern IntersectionEntry intersection_list[MAX_INTERSECTIONS];
extern int total_intersections;

extern TrainRoute train_list[MAX_TRAINS];
extern int total_trains;

// Parsing function declarations
void load_intersections(const char* filename);
void load_train_routes(const char* filename);
void print_parsed_input();

#endif

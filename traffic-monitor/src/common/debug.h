#pragma once

// Here, we define a region of shared memory where the program can set variables dynamically as it executes.
// In case of a segmenation fault, we can inspect the values of these variables

#ifdef __KERNEL__
#define DEBUG_ENTER(state)
#define DEBUG_EXIT(state)
#else

#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#define DEBUG_SHM_NAME "/traffic-monitor-shared"

#define EXECUTION_STATES     \
    ENTRY(FindIpv4Status)    \
    ENTRY(CleanUpTombstones) \
    ENTRY(SetIpv4Status)     \
    ENTRY(GetIpv4Status1)    \
    ENTRY(GetIpv4Status2)

typedef enum
{
#define ENTRY(x) x,
    EXECUTION_STATES
#undef ENTRY
} ExecutionState;


void init_debug(bool init);
void __debug_push_state(ExecutionState state);
void __debug_pop_state(ExecutionState state);
void print_debug_stack();

#define DEBUG_ENTER(state) __debug_push_state(state);
#define DEBUG_EXIT(state) __debug_pop_state(state);

#endif
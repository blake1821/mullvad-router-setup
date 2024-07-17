#include "debug.h"
#ifndef __KERNEL__

static int fd;

struct DebugSharedMemory
{
    ExecutionState stack[256];
    int stack_index;
};

struct DebugSharedMemory *shm;

void init_debug(bool init)
{
    fd = shm_open(DEBUG_SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(struct DebugSharedMemory));
    shm = (struct DebugSharedMemory *)
        mmap(
            0,                                // NULL means kernel chooses address
            sizeof(struct DebugSharedMemory), // Size of the shared memory
            PROT_READ | PROT_WRITE,           // Read and write permissions
            MAP_SHARED,                       // Shared memory
            fd,                               // File descriptor
            0);                               // Offset

    if (init)
    {
        shm->stack_index = 0;
    }
}

void __debug_push_state(ExecutionState state)
{
    shm->stack[shm->stack_index++] = state;
}

void __debug_pop_state(ExecutionState state)
{
    if (shm->stack_index == 0)
    {
        printf("Error: Stack underflow\n");
        exit(1);
    }
    if (shm->stack[shm->stack_index - 1] != state)
    {
        printf("Error: Stack mismatch\n");
        exit(1);
    }
    shm->stack_index--;
}

void print_debug_stack()
{
    printf("Debug Stack:\n");
    for (int i = 0; i < shm->stack_index; i++)
    {
        switch (shm->stack[i])
        {
#define ENTRY(x)         \
    case x:              \
        printf(#x "\n"); \
        break;
            EXECUTION_STATES
#undef ENTRY
        default:
            printf("Unknown\n");
            break;
        }
    }
    printf("End of stack\n");
}

#endif
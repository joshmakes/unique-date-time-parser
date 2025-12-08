#pragma once

#include "constants.h"

#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

// FNV-1a 32-bit constants (official values)
#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U

#define MAX_ENTRIES INT16_MAX
#define SLOT_SIZE 32

typedef struct MemoryArena
{
    uint32_t availableSlots;
    uint32_t usedSlots;
    ssize_t slotSize;
    char *data;
} Arena_t;

typedef enum ArenaErrorCode
{
    SUCCESS = 0,
    INVALID_INDEX,
    BAD_ARGUMENT,
    NO_SLOTS,
    ALREADY_PRESENT,
    NO_MEMORY = 255
} arena_error_code_t;

arena_error_code_t initArena(Arena_t *ptr);

arena_error_code_t allocateArenaSlot(Arena_t *a, const char *string, ssize_t strLength, uint32_t *outIndex);

arena_error_code_t destroyArena(Arena_t *a);

char const *getData(Arena_t *a, uint32_t index);

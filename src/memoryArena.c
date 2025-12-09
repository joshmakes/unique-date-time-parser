#include "memoryArena.h"
#include "constants.h"

#include <stdio.h>
#include <string.h>

/* From https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function */
static uint32_t fnvStringHash(const unsigned char *str)
{
    uint32_t hash = FNV_OFFSET_BASIS;

    while (*str)
    {
        hash ^= (uint32_t)*str++;
        hash *= FNV_PRIME;
    }

    return hash;
}

arena_error_code_t initArena(Arena_t *a)
{
    if (a == NULL)
    {
        printf("Invalid pointer passed to %s, exiting.\r\n", __func__);
        return BAD_ARGUMENT;
    }

    a->availableSlots = MAX_ENTRIES;
    a->usedSlots = 0;
    a->slotSize = SLOT_SIZE;
    a->data = malloc(a->availableSlots * a->slotSize);

    if (a->data == NULL)
    {
        printf("Failed to allocate enough memory (%ld) bytes. Exiting.\r\n", a->availableSlots * a->slotSize);
        return NO_MEMORY;
    }

    memset(a->data, 0, a->slotSize * a->availableSlots);

    return SUCCESS;
}

arena_error_code_t allocateArenaSlot(Arena_t *a, const char *string, ssize_t strLength, uint32_t *outIndex)
{
    if (!a || !string || !outIndex)
    {
        printf("Invalid argument(s) passed into %s.\n", __func__);
        return BAD_ARGUMENT;
    }

    if (!a->data)
        return NO_MEMORY;

    if (a->usedSlots == a->availableSlots)
        return NO_SLOTS;

    if (strLength >= a->slotSize)
    {
        printf("String too long (%zu) for slot size %zu\n",
               strLength, a->slotSize);
        return BAD_ARGUMENT;
    }

    const uint32_t hashVal = fnvStringHash((const unsigned char *)string);
    const uint32_t start = hashVal % a->availableSlots;

    for (uint32_t probe = 0; probe < a->availableSlots; ++probe)
    {
        ssize_t slot = (start + probe) % a->availableSlots;
        char *dest = a->data + (slot * a->slotSize);

        /// Empty slot: insert new string
        if (dest[0] == '\0')
        {
            memcpy(dest, string, strLength);
            dest[strLength] = '\0';
            *outIndex = slot;
            a->usedSlots++;
            return SUCCESS;
        }

        /// Slot used: check if it's the same string
        if (strncmp(dest, string, a->slotSize) == 0)
        {
            *outIndex = slot;
            return ALREADY_PRESENT;
        }
    }

    return NO_SLOTS;
}

char const *getData(Arena_t *a, uint32_t index)
{
    if (a == NULL || a->data == NULL || index == UINT32_MAX)
    {
        perror("Invalid arguments for getData()");
        return NULL;
    }

    // index must be in range [0, availableSlots)
    if (index >= a->availableSlots)
    {
        perror("Invalid index for getData()");
        return NULL;
    }

    return a->data + a->slotSize * index;
}

arena_error_code_t destroyArena(Arena_t *a)
{
    if (a == NULL)
        return BAD_ARGUMENT;

    a->availableSlots = 0;
    a->slotSize = 0;
    a->usedSlots = 0;
    free(a->data);

    return SUCCESS;
}
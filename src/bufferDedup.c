#include "bufferDedup.h"
#include "constants.h"
#include "formatValidator.h"

#include <string.h>
#include <stdio.h>

#include <unistd.h> // write()
#include <fcntl.h>
#include <stdlib.h>    // malloc()
#include <limits.h>    // UINT32_MAX
#include <stdbool.h>   // bool

const char *const DELIMITER = "\n";

static void handleCandidate(const char *candidate, size_t len, Arena_t *arena, int writeFd)
{
    if (!isValidLen(len))
    {
        printf("Skipping candidate of invalid length: %zu\n", len);
        return;
    }

    if (!isValidDateTime(candidate, len))
    {
        printf("Invalid date/time: '%.*s'\n", (int)len, candidate);
        return;
    }

    uint32_t slotIndex = UINT32_MAX;
    arena_error_code_t rv = allocateArenaSlot(arena, candidate, len, &slotIndex);
    if (rv != SUCCESS)
    {
        printf("Failed to allocate slot in Arena_t (error code): %u - for candidate %s\r\n", rv, candidate);
        return;
    }

    char const *stored = getData(arena, slotIndex);

    if (stored)
    {
        if (!(write(writeFd, stored, strlen(stored)) >= 0 && write(writeFd, "\n", 1) >= 0))
        {
            printf("Error writing to file\r\n");
            perror("");
        }
    }
}

int bufferDedup(char buffer[], char carryOverBuffer[], ssize_t *lastLineEnding, Arena_t *arena, int writeFd)
{
    if (lastLineEnding == NULL || *lastLineEnding < 0)
    {
        printf("Invalid lastLineEnding. Returning\r\n");
        return -1;
    }

    bool usedCarryBuffer = false, combinedThisLine = false;
    char *lineEndingPtr = buffer;
    int position = 0, lineSize = 0, totalNewlines = 0;

    while ((lineEndingPtr = strstr(lineEndingPtr, DELIMITER)) != NULL)
    {
        combinedThisLine = false;
        position = (int)(lineEndingPtr - buffer);
        lineSize = position - *lastLineEnding;

        // Extract current line into a temporary buffer
        char currentLineBuffer[lineSize + 1];
        memset(currentLineBuffer, 0, sizeof(currentLineBuffer));
        memcpy(currentLineBuffer, buffer + *lastLineEnding, lineSize);
        size_t currentLineLen = strlen(currentLineBuffer);

        // Skip if obviously too long for either datetime format
        if (currentLineLen > MAX_DATE_TIME_STR_LEN)
        {
            *lastLineEnding = position + 1;
            lineEndingPtr += 1;
            continue;
        }

        size_t carryOverBufferSize = strlen(carryOverBuffer);
        size_t combinedLen = 0;

        // Do we need to stitch carryOver + current to form a full datetime?
        bool canCombineToValidLen =
            ((currentLineLen + carryOverBufferSize) == MIN_DATE_TIME_STR_LEN) ||
            ((currentLineLen + carryOverBufferSize) == MAX_DATE_TIME_STR_LEN);

        if (currentLineLen != MAX_DATE_TIME_STR_LEN &&
            canCombineToValidLen && carryOverBufferSize > 0 &&
            !usedCarryBuffer)
        {
            combinedLen = carryOverBufferSize + currentLineLen;

            char combinedLineBuff[combinedLen + 1];
            memset(combinedLineBuff, 0, combinedLen + 1);
            memcpy(combinedLineBuff, carryOverBuffer, carryOverBufferSize);
            memcpy(combinedLineBuff + carryOverBufferSize, currentLineBuffer, currentLineLen);

            // One place to handle "found a candidate datetime"
            handleCandidate(combinedLineBuff, combinedLen, arena, writeFd);
            combinedThisLine = true;
            usedCarryBuffer = true;
        }

        // Also treat the current line by itself as a candidate
        // if its length matches one of the valid lengths
        if (!combinedThisLine &&
            (currentLineLen == MIN_DATE_TIME_STR_LEN ||
             currentLineLen == MAX_DATE_TIME_STR_LEN))
        {
            handleCandidate(currentLineBuffer, currentLineLen, arena, writeFd);
        }

        *lastLineEnding = position + 1;
        lineEndingPtr += 1;
        totalNewlines++;
    }

    return 0;
}

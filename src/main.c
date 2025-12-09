/// Main driver

#include "bufferDedup.h"
#include "constants.h"
#include "memoryArena.h"

#include <unistd.h>    // read()
#include <fcntl.h>     // O_RDONLY, O_WRONLY
#include <stdlib.h>    // malloc()
#include <sys/types.h> // uint32_t
#include <limits.h>    // UINT32_MAX
#include <stdio.h>     // printf(), perror()
#include <string.h>    // memset(), memcpy()

char *INPUT_DATA_FILE = NULL;
char *OUTPUT_DATA_FILE = NULL;

int main(int argc, char *argv[])
{
    /// Get input and output files from ARGS
    if (argc > 1)
    {
        INPUT_DATA_FILE = argv[1];
        printf("Using input dataset: %s\r\n", INPUT_DATA_FILE);
        if (argc > 2)
        {
            OUTPUT_DATA_FILE = argv[2];
            printf("Using output dataset: %s\r\n", OUTPUT_DATA_FILE);
        }
    }

    /// Open input file, do sanity checks, get readFd etc
    int readFd = open(INPUT_DATA_FILE, O_RDONLY);
    int writeFd = open(OUTPUT_DATA_FILE, O_WRONLY);

    if (readFd < 0 || writeFd < 0)
    {
        printf("result from open: (input file): %d, (output file) %d\r\n", readFd, writeFd);
        perror("");
        return -1;
    }

    Arena_t arena;
    initArena(&arena);

    char buffer[256];
    memset(&buffer, 0, sizeof(buffer));
    ssize_t bytesRead = 0, totalBytesRead = 0, lastLineEnding = 0;
    int totalReads = 0;

    /// Statically-sized carry over buffer in case
    char carryOverBuffer[MAX_DATE_TIME_STR_LEN + 1];
    memset(carryOverBuffer, 0, sizeof(carryOverBuffer));

    while (1)
    {
        bytesRead = read(readFd, buffer, sizeof(buffer) - 1);
        if (bytesRead <= 0)
            break;

        buffer[bytesRead] = '\0';

        bufferDedup(buffer, carryOverBuffer, &lastLineEnding, &arena, writeFd);

        if (lastLineEnding > bytesRead)
            lastLineEnding = bytesRead;

        ssize_t remaining = bytesRead - lastLineEnding;
        if (remaining > 0)
        {
            if (remaining > MAX_DATE_TIME_STR_LEN)
                remaining = MAX_DATE_TIME_STR_LEN;

            memcpy(carryOverBuffer, buffer + lastLineEnding, remaining);
            carryOverBuffer[remaining] = '\0';
        }
        else
            carryOverBuffer[0] = '\0';

        lastLineEnding = 0;
        totalBytesRead += bytesRead;
        totalReads++;
    }

    destroyArena(&arena);

    printf("Number of total bytes read: %lu, and total read() calls: %d\r\n", totalBytesRead, totalReads);
    return 0;
}

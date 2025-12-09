#pragma once

#include "memoryArena.h"

#include <sys/types.h> // uint32_t

int bufferDedup(char buffer[], char carryOverBuffer[], ssize_t *lastLineEnding, Arena_t *arena, int writeFd);

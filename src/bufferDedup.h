#pragma once

#include "memoryArena.h"

int bufferDedup(char buffer[], char carryOverBuffer[], int *lastLineEnding, Arena_t *arena, int writeFd);

# ----------------------------------------
# unique-date-time-parser project makefile
# ----------------------------------------

# --- Configuration Variables ---
# Compiler and flags
INPUT_FILE  = "data/input/generated-date-times.txt"
OUTPUT_FILE = "data/output/validated-date-times.txt"
ARGS = $(INPUT_FILE) $(OUTPUT_FILE)
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

# some-invalid-date-times.txt

# Source and Build Directories
SRC_DIR = src
BUILD_DIR = build

# Executable name
TARGET = unique-date-time-parser

# --- Automatic Variable Generation ---
# Find all .c files in src directory
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Substitutes 'src/' with 'build/' and '.c' with '.o'
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))
# Full path to the final executable target
EXECUTABLE = $(BUILD_DIR)/$(TARGET)

# --- Main Targets ---

# Default target: builds the executable
all: $(BUILD_DIR) $(EXECUTABLE)

# Target to link all object files into the final executable
$(EXECUTABLE): $(OBJS)
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Rule to build the build directory if it doesn't exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Generic rule to compile a .c file from src/ into a .o file in build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# --- Utility Targets ---
# Clean target: remove all generated files
clean:
	@$(RM) -r $(BUILD_DIR)

# Run target
run:
	@$(BUILD_DIR)/$(TARGET) $(ARGS)

diff:
	@echo "Diffing input and output files:"
	@if diff $(INPUT_FILE) $(OUTPUT_FILE) > /dev/null; then \
		echo "✔ No differences detected"; \
	else \
		echo "✘ Differences found:"; \
		diff $(INPUT_FILE) $(OUTPUT_FILE) || true; \
	fi

.PHONY: all clean

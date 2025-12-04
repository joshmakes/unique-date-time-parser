# ----------------------------------------
# unique-date-time-parser project makefile
# ----------------------------------------

# --- Configuration Variables ---
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

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
	@echo "Linking $@"
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# Rule to build the build directory if it doesn't exist
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# Generic rule to compile a .c file from src/ into a .o file in build/
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<"
	$(CC) $(CFLAGS) -I$(SRC_DIR) -c $< -o $@

# --- Utility Targets ---

# Clean target: remove all generated files
clean:
	@echo "Cleaning up..."
	$(RM) -r $(BUILD_DIR)

# Run target
run:
	$(echo "Running $(TARGET)...")
	@$(BUILD_DIR)/$(TARGET)

.PHONY: all clean
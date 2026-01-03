RAYLIB_PATH := src/raylib-5.5

# Compiler and linker
CC := gcc
LD := gcc  # Use gcc to link (easier on Windows)
CFLAGS := -Wall -Wextra -O2 -Iinclude -I$(RAYLIB_PATH)/include
LDFLAGS := -L$(RAYLIB_PATH)/lib -lraylib -lopengl32 -lgdi32 -lwinmm -Wl,-subsystem,console


# Directories
SRC_DIR := src
BUILD_DIR := build

# Files
TARGET := $(BUILD_DIR)/impedancer
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/s2p.c
OBJS := $(BUILD_DIR)/main.o $(BUILD_DIR)/s2p.o

all: $(TARGET)

run: $(TARGET)
	$(TARGET) $(ARGS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

clean:
	del /Q $(BUILD_DIR)\* 2>nul || rm -rf $(BUILD_DIR)/*

.PHONY: all clean

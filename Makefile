# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM := WINDOWS
	RAYLIB_PATH := src/thirdparty/raylib-5.5
	LDFLAGS_PLATFORM := -lopengl32 -lgdi32 -lwinmm -Wl,-subsystem,console
else
    PLATFORM := LINUX
	RAYLIB_PATH := src/thirdparty/raylib-5.5-linux
	LDFLAGS_PLATFORM := -lGL -lm -lpthread -ldl -lrt -lX11
endif

THIRDPARTY_PATH := src/thirdparty

# Compiler and linker
CC := gcc
LD := gcc
CFLAGS := -Wall -Wextra -O2 -Iinclude -I$(RAYLIB_PATH)/include -I$(THIRDPARTY_PATH) -ggdb
LDFLAGS := -L$(RAYLIB_PATH)/lib -lraylib $(LDFLAGS_PLATFORM)


# Directories
SRC_DIR := src
BUILD_DIR := build

# Files
TARGET := $(BUILD_DIR)/impedancer
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/s2p.c $(SRC_DIR)/mui.c $(SRC_DIR)/mma.c $(SRC_DIR)/mui_platform_raylib.c
OBJS := $(BUILD_DIR)/main.o $(BUILD_DIR)/s2p.o $(BUILD_DIR)/mui.o $(BUILD_DIR)/mma.o $(BUILD_DIR)/mui_platform_raylib.o
HEADER_DEPS := $(SRC_DIR)/mui.h $(SRC_DIR)/gra.h

all: $(TARGET)

run: $(TARGET)
	$(TARGET) $(ARGS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER_DEPS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean

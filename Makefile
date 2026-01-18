# Platform detection
ifeq ($(OS),Windows_NT)
    PLATFORM := WINDOWS
	RAYLIB_PATH := src/thirdparty/raylib-5.5
	STATIC_LIBS :=
	LDFLAGS_PLATFORM := -lopengl32 -lgdi32 -lwinmm -Wl,-subsystem,console -lShlwapi
else
    PLATFORM := LINUX
	RAYLIB_PATH := src/thirdparty/raylib-5.5-linux
	STATIC_LIBS := $(RAYLIB_PATH)/lib/libraylib.a
	LDFLAGS_PLATFORM := -lGL -lm -lpthread -ldl -lrt -lX11
endif

THIRDPARTY_PATH := src/thirdparty

# Compiler and linker
CC := gcc
LD := gcc
CFLAGS := -Wall -Wextra -Iinclude -I$(RAYLIB_PATH)/include -I$(THIRDPARTY_PATH) -ggdb
LDFLAGS := $(STATIC_LIBS) -L$(RAYLIB_PATH)/lib -lraylib $(LDFLAGS_PLATFORM)

# Directories
SRC_DIR := src
BUILD_DIR := build

# Files
TARGET := $(BUILD_DIR)/impedancer
SRCS := $(SRC_DIR)/main.c $(SRC_DIR)/s2p.c $(SRC_DIR)/mui.c $(SRC_DIR)/gra.c \
		$(SRC_DIR)/uti.c $(SRC_DIR)/mma.c $(SRC_DIR)/mui_platform_raylib.c \
		$(SRC_DIR)/gra_smithchart.c $(SRC_DIR)/circuit_views.c $(SRC_DIR)/circuit_creation.c

OBJS := $(BUILD_DIR)/main.o $(BUILD_DIR)/s2p.o $(BUILD_DIR)/mui.o $(BUILD_DIR)/gra.o \
		$(BUILD_DIR)/uti.o $(BUILD_DIR)/mma.o $(BUILD_DIR)/mui_platform_raylib.o \
		$(BUILD_DIR)/gra_smithchart.o $(BUILD_DIR)/circuit_views.o $(BUILD_DIR)/circuit_creation.o


HEADER_DEPS :=

all: $(TARGET)

run: $(TARGET)
	$(TARGET) $(ARGS)

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(HEADER_DEPS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(LD) $(OBJS) -o $@ $(LDFLAGS)

tests: $(SRC_DIR)/mma_tests.c $(SRC_DIR)/mma.c
	$(CC) $(CFLAGS) $(SRC_DIR)/mma_tests.c $(SRC_DIR)/mma.c -o $(BUILD_DIR)/tests $(LDFLAGS)
	$(BUILD_DIR)/tests

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean

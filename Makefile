CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O3

TARGET_EXEC := reflex

BUILD_DIR := ./build

SRC_FILES := src/main.c src/token.c src/string_utils.c src/command.c

OBJ_FILES := $(SRC_FILES:%=$(BUILD_DIR)/%.o)

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJ_FILES)
	$(CC) $(OBJ_FILES) -o $@

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -r $(BUILD_DIR)

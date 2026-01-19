CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -Iinclude -g
LDFLAGS =

SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = .

TCP_SRCS = $(wildcard $(SRC_DIR)/tcp/*.c)
LEDGER_SRCS = $(wildcard $(SRC_DIR)/ledger/*.c)
BLOCK_SRCS = $(wildcard $(SRC_DIR)/block/*.c)
CONSENSUS_SRCS = $(wildcard $(SRC_DIR)/consensus/*.c)
UTILS_SRCS = $(wildcard $(SRC_DIR)/utils/*.c)

ALL_SRCS = $(TCP_SRCS) $(LEDGER_SRCS) $(BLOCK_SRCS) $(CONSENSUS_SRCS) $(UTILS_SRCS)

TCP_OBJS = $(TCP_SRCS:$(SRC_DIR)/tcp/%.c=$(OBJ_DIR)/tcp/%.o)
LEDGER_OBJS = $(LEDGER_SRCS:$(SRC_DIR)/ledger/%.c=$(OBJ_DIR)/ledger/%.o)
BLOCK_OBJS = $(BLOCK_SRCS:$(SRC_DIR)/block/%.c=$(OBJ_DIR)/block/%.o)
CONSENSUS_OBJS = $(CONSENSUS_SRCS:$(SRC_DIR)/consensus/%.c=$(OBJ_DIR)/consensus/%.o)
UTILS_OBJS = $(UTILS_SRCS:$(SRC_DIR)/utils/%.c=$(OBJ_DIR)/utils/%.o)

OBJS = $(TCP_OBJS) $(LEDGER_OBJS) $(BLOCK_OBJS) $(CONSENSUS_OBJS) $(UTILS_OBJS)

TARGET = $(BIN_DIR)/ledger_tcp_chain

.PHONY: all clean fclean re

all: $(TARGET)

$(TARGET): $(OBJS) $(OBJ_DIR)/main.o
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

$(OBJ_DIR)/tcp/%.o: $(SRC_DIR)/tcp/%.c
	@mkdir -p $(OBJ_DIR)/tcp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/ledger/%.o: $(SRC_DIR)/ledger/%.c
	@mkdir -p $(OBJ_DIR)/ledger
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/block/%.o: $(SRC_DIR)/block/%.c
	@mkdir -p $(OBJ_DIR)/block
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/consensus/%.o: $(SRC_DIR)/consensus/%.c
	@mkdir -p $(OBJ_DIR)/consensus
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/utils/%.o: $(SRC_DIR)/utils/%.c
	@mkdir -p $(OBJ_DIR)/utils
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/main.o: main.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	@echo "Object files cleaned"

fclean: clean
	rm -f $(TARGET)
	@echo "Executable cleaned"

re: fclean all

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET)

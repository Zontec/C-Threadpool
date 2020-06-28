PROJECT=test


CC=gcc
LD=gcc

BUILD_DIR=build
BIN_DIR=bin
SRC_DIR=src

EXEC=$(BIN_DIR)/$(PROJECT)

SRCS = test.c
SRCS += $(SRC_DIR)/interlocked.c
SRCS += $(SRC_DIR)/threadpool.c

OBJECTS=$(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))

CFLAGS = -std=c11 -w -D__LOG -D__DEBUG -D__LSB -pthread 
LFLAGS = -std=c11 -w -pthread

INCLUDE = src/

all:$(EXEC)
	@echo Build done: $(EXEC)!

$(EXEC):$(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(LD) -I$(INCLUDE) $(LFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -I$(INCLUDE) $(CFLAGS) -c $< -o $@


clean: 
	@rm -rf $(BIN_DIR)
	@rm -rf $(BUILD_DIR)
	@echo Clean done!

rebuild: clean
	@make

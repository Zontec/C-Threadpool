PROJECT=test


CC=gcc
LD=gcc

BUILD_DIR=build
BIN_DIR=bin
SRC_DIR=src
INCLUDE = src/
EXEC=$(BIN_DIR)/$(PROJECT)

##MACRO
DEBUG?=NO_DEBUG
LOG?=NO_LOG
ENDI?=__LSB

##SRCS



SRCS = test.c
SRCS += $(SRC_DIR)/threadpool.c

##OBJECTS
OBJECTS=$(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))


CFLAGS = -std=c11 -w -D$(LOG) -D$(DEBUG) -D$(ENDI) -pthread 

LFLAGS = -std=c11 -w -pthread


all:$(EXEC)
	@echo Build done: $(EXEC)!

$(EXEC):$(OBJECTS)
	mkdir -p $(BIN_DIR)
	$(LD) -I$(INCLUDE) $(DEBUG_FLAG) $(LFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) -I$(INCLUDE) $(DEBUG_FLAG) $(CFLAGS) -c $< -o $@

debug:
	DEBUG_FLAG=-g LOG=__LOG DEBUG=__DEBUG make rebuild

clean: 
	@rm -rf $(BIN_DIR)
	@rm -rf $(BUILD_DIR)
	@echo Clean done!

rebuild: clean
	@make

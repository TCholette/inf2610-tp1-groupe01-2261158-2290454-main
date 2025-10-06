CC = gcc
CFLAGS = -Wall -Wextra -O0 -Iutils
LDFLAGS = -pthread

SRC ?= section1/section1.c
BIN_DIR = bin
EXEC = $(BIN_DIR)/$(notdir $(basename $(SRC)))

UTILS = utils/timer.c

LOG1 ?= utils/logs.txt
LOG2 ?= utils/logs_2.txt
MOTS ?= utils/mots.txt
N ?= 3

all: $(EXEC)
ifeq ($(notdir $(basename $(SRC))),section1)
	./$(EXEC) $(MOTS)
endif

ifeq ($(notdir $(basename $(SRC))),section2)
	./$(EXEC) $(LOG1) $(LOG2) $(N)
endif

ifeq ($(notdir $(basename $(SRC))),section3)
	./$(EXEC) $(LOG1) $(LOG2) $(N)
endif

$(EXEC): $(SRC) $(UTILS)
	rm -rf $(BIN_DIR)
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(SRC) $(UTILS) -o $(EXEC) $(LDFLAGS)

clean:
	rm -f $(BIN_DIR)/*


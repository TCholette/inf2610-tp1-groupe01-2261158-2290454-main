# Variables par défaut
CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC ?= section2.c          # Par défaut 
EXEC = $(basename $(SRC))
LOG1 ?= logs.txt
LOG2 ?= logs_2.txt
N ?= 3


all: $(EXEC)
	./$(EXEC) $(LOG1) $(LOG2) $(N)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

clean:
	rm -f *.o $(EXEC) RESULT_PROCESS.txt


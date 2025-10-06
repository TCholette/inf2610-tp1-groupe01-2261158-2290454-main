# Variables par défaut
CC = gcc
CFLAGS = -Wall -Wextra -O2
SRC ?= section1.c          # Par défaut 
EXEC = $(basename $(SRC))
LOG ?= mots.txt

all: $(EXEC)
	./$(EXEC) $(LOG1) $(LOG2)

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $(EXEC) $(SRC)

clean:
	rm -f *.o $(EXEC) RESULT_PROCESS.txt


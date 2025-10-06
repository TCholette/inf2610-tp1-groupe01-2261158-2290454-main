CC = gcc
CFLAGS = -Wall -Wextra -O2 -pthread
SRC = section3.c
TARGET = exe

all: $(TARGET)
	./$(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET) *.o RESULT_THREADS.txt

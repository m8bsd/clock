CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = webserver
SOURCE = webserver.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99

# Target executable
TARGET = beacon-flood

# Source and header files
SRC = beacon-flood.c
HEADER = beacon-flood.h

# Build target
$(TARGET): $(SRC) $(HEADER)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) -lrt

# Clean build artifacts
clean:
	rm -f $(TARGET)
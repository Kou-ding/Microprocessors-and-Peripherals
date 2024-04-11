# Compiler
CC = gcc
# Compiler flags
CFLAGS = -Wall 

# Target executable
TARGET = test

# Default target
all: $(TARGET)

# Compile source files into object files
$(TARGET): lab1.c
	$(CC) $(CFLAGS) $< -o $@

# Clean up object files and the target executable
clean:
	rm -f $(OBJS) $(TARGET)
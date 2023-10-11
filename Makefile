CC = gcc
#CFLAGS = -Wall -Wextra -std=c99

# Define the source directory, source files, and object files
SRC_DIR = clox
SRC = $(wildcard $(SRC_DIR)/*.c) 
OBJ = $(SRC:$(SRC_DIR)/%.c=%.o) 

# Define header dependencies
DEPS = $(wildcard $(SRC_DIR)/*.h) 

# The default target is to build the "main" executable
all: lox

# Compile each source file into object files
%.o: $(SRC_DIR)/%.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Link the object files to create the "main" executable
lox: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

# Clean up intermediate object files
clean:
	rm -f $(OBJ) lox

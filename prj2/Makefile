## This is a simple Makefile with lost of comments 
## Check Unix Programming Tools handout for more info.

# Define what compiler to use and the flags.
CC=gcc
CXX=g++
CCFLAGS= -g -Wall -Werror

all: shell_sol

# Compile all .c files into .o files
# % matches all (like * in a command)
# $< is the source file (.c file)
%.o : %.c
	$(CC) -c $(CCFLAGS) $<



shell: shell.o 
	$(CC) -o shell shell.o $(CCFLAGS)

shell_sol: shell_sol.o 
	$(CC) -o shell_sol shell_sol.o $(CCFLAGS)

clean:
	rm -f core *.o shell shell_sol


#  Filename: Makefile
#  Created by Ivan Jonathan Hoo
#  Copyright (c) 2016 Ivan Jonathan Hoo. All rights reserved.

CC = gcc
CCFLAGS = -g -std=c99 -w

SRC = shell.c

all: shell

shell: $(SRC)
	$(CC) $(CCFLAGS) $(SRC) -o shell

clean:
	rm -f shell *.o
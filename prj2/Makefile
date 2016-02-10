#  Filename: Makefile
#  Created by Ivan Jonathan Hoo
#  Copyright (c) 2016 Ivan Jonathan Hoo. All rights reserved.

CC = gcc
CFLAGS = -std=c99 -g 
LDFLAGS += -lm -lglut -lGL

SRC = shell.c

all: shell

shell: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o shell $(LDFLAGS)

clean:
	rm -f shell *.o
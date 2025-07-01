CC = gcc
CFLAGS = -Wextra -Wall
EXEC = build/jank
SRCS = $(wildcard src/*.c)

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC)
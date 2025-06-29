CC = gcc
CFLAGS = -Wextra -Wall
EXEC = build/a
SRCS = $(wildcard src/*.c)

all:
	$(CC) $(CFLAGS) $(SRCS) -o $(EXEC)
CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=gnu99
LDFLAGS = -L. -lcrc
INC = -I.

main: main.c checksum.h
	$(CC) $(CFLAGS) $(INC) -o main main.c $(LDFLAGS)

.PHONY: clean
clean:
	rm -f main

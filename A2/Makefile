CC := gcc
CFLAGS := -W -Wall -Wextra -pedantic -std=gnu99 -g -O2 -pipe
TARGET := tcpchatclientserver
OBJ := server.o client.o protocol.o linked_list.o

build: all

all: $(OBJ) main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(OBJ)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(TARGET)

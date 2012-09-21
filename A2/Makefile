CC := gcc
CFLAGS := -W -Wall -Wextra -pedantic -std=gnu99 -g -O2
TARGET := tcpradioclientserver
OBJ := common.o server.o client.o protocol.o linked_list.o

build: all

all: $(OBJ) main.c
	$(CC) $(CFLAGS) main.c -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(TARGET)

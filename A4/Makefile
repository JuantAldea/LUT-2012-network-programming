####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                     Makefile                     #
####################################################

CC := gcc
CFLAGS := -Wall -pedantic -std=gnu99 -g -pipe
SERVER := gameclient
OBJ :=  protocol.o linked_list.o

build: all

all: $(SERVER)

$(SERVER): $(OBJ) gameclient.o
	$(CC) $(CFLAGS) -o $(SERVER) $(OBJ) gameclient.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(SERVER)

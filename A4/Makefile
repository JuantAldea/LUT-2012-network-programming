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
SERVER := aphorismserver
CLIENT := aphorismclient
OBJ :=  protocol.o linked_list.o common.o

build: all

all: $(SERVER) $(CLIENT)

$(SERVER): $(OBJ) server.o
	$(CC) $(CFLAGS) -o $(SERVER) $(OBJ) server.o

$(CLIENT): $(OBJ) client.o
	$(CC) $(CFLAGS) -o $(CLIENT) $(OBJ) client.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(SERVER) $(CLIENT)

###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################


CC := gcc
CFLAGS := -W -Wall -Wextra -pedantic -std=gnu99 -g -O2 -pipe
SERVER := aphorismserver
CLIENT := aphorismclient
OBJ :=  protocol.o linked_list.o

build: all

all: $(SERVER) $(CLIENT)

$(SERVER): $(OBJ) server.o
	$(CC) $(CFLAGS) -o $(TARGET) server.c $(OBJ) server.o

$(CLIENT): $(OBJ) client.o
	$(CC) $(CFLAGS) -o $(CLIENT) client.c $(OBJ) client.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(SERVER) $(CLIENT)

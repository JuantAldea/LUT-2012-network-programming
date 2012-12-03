###############################################
#        CT30A5001 - Network Programming      #
#               Home Examination              #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################


CC := gcc
CFLAGS := -W -Wall -Wextra -pedantic -std=gnu99 -g -pipe
TARGET1 := server
TARGET2 := game
OBJ := linked_list.o map.o md5.o tcp.o map_server.o udp.o game_server.o chat_server.o game_protocol.o chat_protocol.o

build: all

all: $(TARGET1) $(TARGET2)

$(TARGET1): $(OBJ) server.c
	$(CC) $(CFLAGS) -o $(TARGET1) server.c $(OBJ) -lncurses
$(TARGET2): $(OBJ) nc.c
	$(CC) $(CFLAGS) -o $(TARGET2) nc.c udp.o tcp.o map.o md5.o game_protocol.o chat_protocol.o -lncurses

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(TARGET1) $(TARGET2)

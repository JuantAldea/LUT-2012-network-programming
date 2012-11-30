###############################################
#        CT30A5001 - Network Programming      #
#          Assignment 6: FTP Client           #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################


CC := gcc
CFLAGS := -W -Wall -Wextra -pedantic -std=gnu99 -g -pipe
TARGET := server
OBJ := linked_list.o map.c md5.c tcp.c map_server.c udp.c game_server.c chat_server.c

build: all

all: $(TARGET)
	gcc -Wall -g -pedantic -std=gnu99 -lncurses nc.c -o game

$(TARGET): $(OBJ) server.c
	$(CC) $(CFLAGS) -o $(TARGET) server.c $(OBJ)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(TARGET)
	rm -f game


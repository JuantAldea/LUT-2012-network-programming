###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################


CC := gcc
CFLAGS := -W -Wall -Wextra -pedantic -std=gnu99 -g -O2 -pipe
TARGET := tcpchatclientserver
OBJ := client.o protocol.o

build: all

all: $(TARGET)

$(TARGET): $(OBJ) main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c $(OBJ)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $*.c -o $*.o

clean:
	rm -f *.o $(TARGET)

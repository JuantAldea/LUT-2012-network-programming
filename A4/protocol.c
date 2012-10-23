/*
####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                    protocol.c                    #
####################################################
*/
#include <math.h>
#include "protocol.h"

int send_msg(int socket, struct sockaddr *addr, socklen_t address_len, uchar *msg, int8_t msg_size)
{
	int bytes_to_send = sizeof(uchar) * msg_size;
	return sendto(socket, msg, bytes_to_send, 0, addr,address_len);
}

int recv_msg(int socket, struct sockaddr *addr, socklen_t *address_len, char *buffer)
{
	int recv_bytes = recvfrom(socket, &buffer, 512, 0, addr, address_len);
	printf("RECV %3s\n", buffer);
	return recv_bytes;
}

int send_WHOIS(int socket, struct sockaddr *addr, socklen_t address_len)
{
	uint8_t msg = 0x0;
	return send_msg(socket, addr, address_len, (uchar *)&msg, 3);
}

int send_HOSTINFO(int socket, struct sockaddr *addr, socklen_t address_len, uint16_t port)
{
	uchar *msg = (uchar *)malloc(sizeof(uint8_t) + sizeof(uint16_t));
	((uint8_t *)msg[0]) = 0x1;
	((uint8_t *)msg[1]) = htons(port);
	return send_msg(socket, addr, address_len, msg, sizeof(uint8_t)  + sizeof(uint16_t));
}

int send_HELLO(int socket, struct sockaddr *addr, socklen_t address_len)
{
	uint8_t msg = 0x2;
	return send_msg(socket, addr, address_len, (uchar*)&msg, sizeof(uint8_t));
}

int send_HREPLY(int socket, struct sockaddr *addr, socklen_t address_len)
{
	uint8_t msg = 0x03;
	return send_msg(socket, addr, address_len, (uchar*)&msg, sizeof(uint8_t));
}

int send_POSITION(int socket, struct sockaddr *addr, socklen_t address_len, uint8_t x, uint8_t y)
{
	uint8_t msg[3];
	msg[0] = 0x4;
	msg[1] = x;
	msg[2] = y;
	return send_msg(socket, addr, address_len, (uchar *)msg, 3 * sizeof(uint8_t));
}

int send_GRID(int socket, struct sockaddr *addr, socklen_t address_len, char grid[9])
{
	int8_t msg_length = sizeof(char) * 9 + sizeof(uint8_t);
	uchar *msg = (uchar*)malloc(msg_length);
	(*((uint8_t *)msg)) = 0x5;
	char *ptr = (char *)((uint8_t*)msg + 1);
	memcpy(ptr, grid, sizeof(grid));
	return send_msg(socket, addr, address_len, msg, msg_length);
}

int send_WINNER(int socket, struct sockaddr *addr, socklen_t address_len, char player_mark)
{
	uchar *msg = (uchar *)malloc(sizeof(char) + sizeof(uint8_t));
	(*(uint8_t*)msg) = 0x6;
	((uint8_t *)msg)[1] = player_mark;
	return send_msg(socket, addr, address_len, msg, sizeof(char) + sizeof(uint8_t));
}

int send_QUIT(int socket, struct sockaddr *addr, socklen_t address_len)
{
	uint8_t msg = 0x7;
	return send_msg(socket, addr, address_len, (uchar *)&msg, sizeof(uint8_t));
}

int send_ERROR(int socket, struct sockaddr *addr, socklen_t address_len, uint16_t error_type)
{
	uchar *msg = (uchar *)malloc(sizeof(uint8_t) + sizeof(uint16_t));
	*(uint8_t *)msg = 0x8;
	*(uint16_t *)((uint8_t *)msg + 1) = htons(error_type);
	return send_msg(socket, addr, address_len, msg, sizeof(uint8_t) + sizeof(uint16_t)));
}

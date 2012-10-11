/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 protocol.c                  #
###############################################
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

int send_AOK(int socket, struct sockaddr *addr, socklen_t address_len)
{
	uchar msg[] = "AOK";
	return send_msg(socket, addr, address_len, msg, 3);
}

int send_ERR(int socket, struct sockaddr *addr, socklen_t address_len, char *msg)
{
	uchar full_msg[512];
	int msg_size = snprintf((char*)full_msg, 512, "ERR%s", msg) + 1;
	return send_msg(socket, addr, address_len, full_msg, msg_size < 512 ? msg_size : 512);
}

int send_APH(int socket, struct sockaddr *addr, socklen_t address_len, char *msg)
{
	uchar full_msg[512];
	int msg_size = snprintf((char*)full_msg, 512, "APH%s", msg) + 1;
	return send_msg(socket, addr, address_len, full_msg, msg_size < 512 ? msg_size : 512);
}

int send_ADD(int socket, struct sockaddr *addr, socklen_t address_len, char *msg)
{
	uchar full_msg[512];

	int msg_size = snprintf((char*)full_msg, 512, "ADD%s", msg) + 1;
	return send_msg(socket, addr, address_len, full_msg, msg_size < 512 ? msg_size : 512);
}

int send_GET(int socket, struct sockaddr *addr, socklen_t address_len)
{
	uchar msg[] = "GET";
	return send_msg(socket, addr, address_len, msg, 3);
}

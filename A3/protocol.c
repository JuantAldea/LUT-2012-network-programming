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

int send_msg(int socket, struct addrinfo *addr, uchar *msg, int8_t msg_size)
{
	int bytes_to_send = sizeof(uchar) * msg_size;
	int total_sent_bytes = 0;
	while (total_sent_bytes < bytes_to_send){
		total_sent_bytes += sendto(socket,
								   &(msg[total_sent_bytes]),
								   bytes_to_send - total_sent_bytes,
								   0,
								   addr->ai_addr,
								   addr->ai_addrlen
						   );
	}
	return total_sent_bytes;
}

int recv_msg(int socket, struct addrinfo *addr, char *buffer)
{
	int recv_bytes = recvfrom(socket, &buffer, 512, 0, addr->ai_addr, &addr->ai_addrlen);
	return recv_bytes;
}

int send_AOK(int socket, struct addrinfo *addr)
{
	uchar msg[] = "AOK";
	return send_msg(socket, addr, msg, 3);
}

int send_ERROR(int socket, struct addrinfo *addr, char *msg)
{
	uchar full_msg[512];
	int msg_size = snprintf((char*)full_msg, 512, "ERR%s", msg) + 1;
	return send_msg(socket, addr, full_msg, msg_size < 512 ? msg_size : 512);
}

int send_APH(int socket, struct addrinfo *addr, char *msg)
{
	uchar full_msg[512];
	int msg_size = snprintf((char*)full_msg, 512, "APH%s", msg) + 1;
	return send_msg(socket, addr, full_msg, msg_size < 512 ? msg_size : 512);
}

int send_ADD(int socket, struct addrinfo *addr, char *msg)
{
	uchar full_msg[512];
	int msg_size = snprintf((char*)full_msg, 512, "ADD%s", msg) + 1;
	return send_msg(socket, addr, full_msg, msg_size < 512 ? msg_size : 512);
}

int send_GET(int socket, struct addrinfo *addr)
{
	uchar msg[] = "GET";
	return send_msg(socket, addr, msg, 3);
}

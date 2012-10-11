/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                   protocol.h                #
###############################################
*/

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stropts.h>
#include <sys/ioctl.h>
#include "protocol_constants.h"
#include "common.h"
#include "recv_buffer.h"
#include "linked_list.h"
#include  "common.h"

#define HEADER_SIZE 6

int send_msg(int socket, struct addrinfo *addr, uchar *msg, int8_t msg_size);
int recv_msg(int socket, struct addrinfo *addr, char *buffer);

int send_AOK(int socket, struct addrinfo *addr);
int send_ERR(int socket, struct addrinfo *addr, char *msg);

int send_APH(int socket, struct addrinfo *addr, char *msg);
int send_ADD(int socket, struct addrinfo *addr, char *msg);
int send_GET(int socket, struct addrinfo *addr);


#endif

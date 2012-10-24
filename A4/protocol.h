/*
####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                     protocol.h                   #
####################################################
*/

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stropts.h>
#include <sys/ioctl.h>
#include "common.h"
#include "linked_list.h"
#include  "common.h"

#define WHOIS 0x0
#define HOSTINFO 0x1
#define HELLO 0x2
#define HREPLY 0x3
#define POSITION 0x4
#define GRID 0x5
#define WINNER 0x6
#define QUIT 0x7
#define ERROR 0x8

int send_msg(int socket, struct sockaddr *addr, socklen_t address_len, uchar *msg, int8_t msg_size);
int recv_msg(int socket, struct sockaddr *addr, socklen_t *address_len, char *buffer);

int send_WHOIS(int socket, struct sockaddr *addr, socklen_t address_len);
int send_HOSTINFO(int socket, struct sockaddr *addr, socklen_t address_len, uint16_t port);
int send_HELLO(int socket, struct sockaddr *addr, socklen_t address_len);
int send_HREPLY(int socket, struct sockaddr *addr, socklen_t address_len);
int send_POSITION(int socket, struct sockaddr *addr, socklen_t address_len, uint8_t x, uint8_t y);
int send_GRID(int socket, struct sockaddr *addr, socklen_t address_len, char grid[9]);
int send_WINNER(int socket, struct sockaddr *addr, socklen_t address_len, char player_mark);
int send_QUIT(int socket, struct sockaddr *addr, socklen_t address_len);
int send_ERROR(int socket, struct sockaddr *addr, socklen_t address_len, uint16_t error_type);

#endif

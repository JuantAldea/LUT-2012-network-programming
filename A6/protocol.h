/*
###############################################
#        CT30A5001 - Network Programming      #
#            Assignment 6: FTP client         #
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

#define MAX_MSG_SIZE 2048

int send_msg(int socket, uchar *msg, int msg_size);
int recv_msg(int socket, char *buffer);

int send_anonymous_login(int socket);
int send_login(int socket, char *username, char *password);

int send_username(int socket, char *username);
int send_password(int socket, char *password);

void dump_msg(uchar *msg, int length);

#endif

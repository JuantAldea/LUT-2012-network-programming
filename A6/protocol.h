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
#include "common.h"

#define MAX_MSG_SIZE 2048

int send_msg(int socket, uchar *msg, int msg_size);
int recv_msg(int socket, char *buffer);

int send_username(int socket, char *username);
int send_password(int socket, char *password);

int send_quit(int socket);
int send_list(int socket);
int send_cwd(int socket);
int send_help(int socket);
int send_cd(int socket, char *path);
int send_put(int socket, char *path);
int send_get(int socket, char *path);

int enter_passive_mode(int socket);

void recv_ls(int socket);
int recv_file(int socket, char *path);
void send_file(int socket, char *path);

void dump_msg(uchar *msg, int length);

#endif

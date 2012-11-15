/*
###############################################
#        CT30A5001 - Network Programming      #
#          Assignment 6: FTP Client           #
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
int send_cd(int socket, char *path);
int send_put(int socket, char *path);
int send_get(int socket, char *path);
int send_binary(int socket);

int send_help(int socket);
int send_syst(int socket);
int send_stat(int socket);
int send_raw(int socket, char *msg);

int enter_passive_mode(int socket);
int enter_active_mode(int socket, int ip[4], int port[2]);

int recv_ls(int socket);
int recv_file(int socket, char *path);
int send_file(int socket, char *path);

void dump_msg(uchar *msg, int length);
float transfer_rate(int size, struct timeval *before, struct timeval *after);
#endif

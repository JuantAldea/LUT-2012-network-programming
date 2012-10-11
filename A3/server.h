/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  server.h                   #
###############################################
*/

#ifndef __SERVER_H__
#define __SERVER_H__

#include "common.h"
#include "server_commands.h"
#include "linked_list.h"

#define BACKLOG 100

int server(char *port);

int prepare_server (char *port);

int parse_command(char *command);

void flush_stdin(void);

int is_number(char *str, int base, int *number);
void help(char *name);

#endif

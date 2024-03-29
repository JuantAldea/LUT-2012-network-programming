/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 client.h                    #
###############################################
*/

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "common.h"
#include "client_commands.h"

int client(char *name, char *address, char *port);
int prepare_connection(char *address, char *port);

int client_parse_command(char *command);

void print_chat_message(recv_buffer_t *buffer);
void print_introduction_message(recv_buffer_t *buffer);
void print_user_list(recv_buffer_t *buffer);
void print_message(recv_buffer_t *buffer);

#endif

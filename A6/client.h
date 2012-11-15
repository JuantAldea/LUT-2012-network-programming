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
#include "client_states.h"

int client_CLI(char **msg);

int client(char *address, char *port);

int prepare_connection(char *address, char *port);

int open_active_mode_server(int control_socket, int ip[4], int port[2]);

int send_mode(int socket_control, int *socket_transfer, enum transfer_modes transfer_mode);

int client_parse_command(char *command);

int read_greetings(int socket);

void ask_login(char **username, char **password);

void reset_client(int *socket_control, int *socket_active, int *socket_transfer, enum states *client_state);

#endif

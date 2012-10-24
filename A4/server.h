/*
####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                    server.h                      #
####################################################
*/

#ifndef __SERVER_H__
#define __SERVER_H__

#include "common.h"
#include "server_commands.h"
#include "linked_list.h"

typedef enum
{
    IDLE,
    PLAYING
}GAME_STATE;


int server(char *port);

int prepare_server (char *port, uint16_t *port_number);

int parse_command(char *command);

void flush_stdin(void);

int is_number(char *str, int base, int *number);

void help(char *name);

void cli();

int message_management(linked_list_t *game_list);

uint16_t get_port(struct sockaddr *sa);

#endif

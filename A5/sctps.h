/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              sctps.h               #
####################################################
*/

#ifndef __SCTP_TESTSERVER_H_
#define __SCTP_TESTSERVER_H_

// Length for message buffer
#define MBUFFERLEN 1024*6

#define ONLYRUNNING 1
#define NOLOOPBACK 0

#include "addresses.h"
#include "game.h"
#include "linked_list.h"
#include "protocol.h"

int server(int argc, char* argv[]);

void sighandler(int);

int testserver_input_error(char*);

int broadcast_start(int sctp_socket, int8_t columns, int8_t rows, linked_list_t *list);

node_t *next_turn_player(node_t *current_player, linked_list_t *players);
int send_turn_to_player(int socket, node_t *player);
int broadcast_winner(int sctp_socket, int8_t winner_id, linked_list_t *list);
int drop_all(int sctp_socket, linked_list_t *list);
int broadcast_area(int sctp_socket, char *area, int8_t rows, int8_t columns, linked_list_t *list);

#endif

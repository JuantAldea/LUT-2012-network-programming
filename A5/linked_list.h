/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                   linked_list.h                  #
####################################################
*/

#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include "addresses.h"
#define uchar unsigned char

typedef struct node_s node_t;
struct node_s {
    struct sockaddr_storage sa;
    sctp_assoc_t session_id;
    int8_t player_id;
    int8_t ready;
    int8_t turn;

    node_t *next;
    node_t *previous;
};

typedef struct linked_list_s linked_list_t;
struct linked_list_s {
    node_t *head;
    node_t *tail;
    int count;
};

void list_init(linked_list_t *list);
void list_clear(linked_list_t *list);

void list_add_first(node_t *n, linked_list_t *list);
void list_add_last (node_t *n, linked_list_t *list);

void list_delete(linked_list_t *list);
void list_remove_node(node_t *n, linked_list_t *list);

node_t *list_create_node(int8_t player_id, sctp_assoc_t session_id, struct sockaddr_storage sa);

void list_print(linked_list_t *list);
void list_reverse_print(linked_list_t *list);

node_t *list_get_node_by_index(int index, linked_list_t *list);

node_t *list_get_node_by_player_id(int8_t player_id, linked_list_t *list);

node_t *list_get_node_by_session_id(sctp_assoc_t session_id, linked_list_t *list);

int8_t list_get_first_free_player_id(linked_list_t *list);

void list_sort_by_turn(linked_list_t *list);

int list_ready_check(linked_list_t *list);

void list_set_movement_order(linked_list_t *player_list);

#endif

/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  linked_list.h              #
###############################################
*/

#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "system_headers.h"
#define uchar unsigned char

typedef struct node_s node_t;
struct node_s {
	void *data;
	node_t *next;
	node_t *previous;
};

typedef struct linked_list_s linked_list_t;
struct linked_list_s {
	node_t *head;
	node_t *tail;
	int count;
};


typedef struct player_info_s player_info_t;

struct player_info_s {
    struct sockaddr_storage addr;
    socklen_t addr_len;
    int chat_descriptor;
    int playerID;
    int current_health;
    int position[2];
    int frags;
    int deaths;
    int death;
    struct timeval death_time;
    struct timeval last_action;
    uint16_t last_udp_package;
};

void list_init(linked_list_t *list);

void list_add_first(node_t *n, linked_list_t *list);
void list_add_last (node_t *n, linked_list_t *list);

void list_delete(linked_list_t *list);
void list_remove_node(node_t *n, linked_list_t *list);

node_t *list_create_node(void *data);

void list_print(linked_list_t *list);
void list_reverse_print(linked_list_t *list);


node_t *list_search_by_addrinfo(struct sockaddr_storage *addr, linked_list_t *list);
node_t *list_search_by_addr(struct sockaddr_storage *addr, linked_list_t *list);
node_t *list_search_by_playerID(uint8_t id, linked_list_t *list);

#endif

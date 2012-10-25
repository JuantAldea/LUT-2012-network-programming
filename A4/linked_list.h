/*
####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
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

#define uchar unsigned char

typedef struct node_s node_t;
struct node_s {
    struct sockaddr_storage *addr;
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

node_t *list_create_node(struct sockaddr_storage *addr);

void list_print(linked_list_t *list);
void list_reverse_print(linked_list_t *list);

node_t *list_get_node_by_index(int index, linked_list_t *list);

#endif

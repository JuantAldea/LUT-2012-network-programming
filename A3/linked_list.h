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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define uchar unsigned char

typedef struct node_s node_t;
struct node_s {
	time_t date_time;
	char *aphorism;
	char *ip;
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

void list_add_first(node_t *n, linked_list_t *list);
void list_add_last (node_t *n, linked_list_t *list);

void list_delete(linked_list_t *list);
void list_remove_node(node_t *n, linked_list_t *list);

node_t *list_create_node(char *ip, char *aphorism);

void list_print(linked_list_t *list);
void list_reverse_print(linked_list_t *list);

node_t *list_get_node_by_index(int index, linked_list_t *list);

#endif

#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <assert.h>
#include "recv_buffer.h"
#define uchar unsigned char
#define MAX_NICKNAME_LENGTH 15

typedef struct client_s client_t;
struct client_s{
	int fd;
	char *name;
};

typedef struct node_s node_t;
struct node_s {
	recv_buffer_t *buffer;
	client_t *client;
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
void list_remove_by_fd(int fd, linked_list_t *list);
void list_remove_by_name(char *name, linked_list_t *list);

node_t *list_create_node(int fd, char *name);

void list_set_name_by_fd(int fd, char *name, linked_list_t *list);
void list_set_name_by_node(node_t *user, char *name, linked_list_t *list);

node_t* list_get_node_by_fd(int fd, linked_list_t *list);
node_t* list_get_node_by_name(char *name, linked_list_t *list);

void list_print(linked_list_t *list);
void list_reverse_print(linked_list_t *list);

recv_buffer_t *list_get_buffer_by_fd(linked_list_t *list);

void client_free(client_t *client);
void recv_buffer_free(recv_buffer_t *buffer);
void recv_buffer_reset(recv_buffer_t *buffer);

#endif

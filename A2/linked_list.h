#ifndef __LINKED_LIST_H__
#define __LINKED_LIST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <assert.h>
#define uchar unsigned char

typedef struct node node;

struct node {
	char *name;
	int fd;
	node *next;
	node *previous;
};

typedef struct buffer buffer;
struct buffer{
	uchar *buff;
	uint8_t type;
	uint16_t length;
};

typedef struct linked_list linked_list;
struct linked_list {
	node *head;
	node *tail;
	int count;
};

void list_init(linked_list *list);

void list_add_first(node *n, linked_list *list);
void list_add_last (node *n, linked_list *list);

void list_delete(linked_list *list);
node *list_remove_node(node *n, linked_list *list);
node *list_remove_by_fd(int fd, linked_list *list);
node *list_remove_by_name(char *name, linked_list *list);

buffer *list_get_buffer_by_fd(linked_list *list);
void list_print(linked_list *list);
void list_reverse_print(linked_list *list);

node *list_create_node(int fd, char *name);

void list_set_name_by_fd(int fd, char *name, linked_list *list);
void list_set_name_by_node(node *user, char *name, linked_list *list);

node* list_get_node_by_fd(int fd, linked_list *list);
node* list_get_node_by_name(char *name, linked_list *list);

#endif
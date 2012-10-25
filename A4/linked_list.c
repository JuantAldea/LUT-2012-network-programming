/*
####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                  linked_list.c                   #
####################################################
*/

#include "linked_list.h"

//set-up the head and tail of the list
void list_init(linked_list_t *list)
{
	list->count = 0;
	list->head = (node_t *)malloc(sizeof(node_t));
	list->tail = (node_t *)malloc(sizeof(node_t));
	list->head->next = list->tail;
	list->head->previous = NULL;
	list->tail->next = NULL;
	list->tail->previous = list->head;
}

//free the list
void list_delete(linked_list_t *list)
{
	list_clear(list);
	free(list->head);
	free(list->tail);
}

void list_clear(linked_list_t *list)
{
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		node_t *previous = i->previous;
		list_remove_node(i, list);
		i = previous;
	}
}

//build a new list node
node_t *list_create_node(struct sockaddr_storage *addr)
{
	node_t *node = (node_t *)malloc(sizeof(node_t));
	node->addr = addr;
	node->next = NULL;
	node->previous = NULL;
	return node;
}

//add node after the head
void list_add_first(node_t *node, linked_list_t *list)
{
	list->count++;
	node->next = list->head->next;
	node->previous = list->head;
	list->head->next->previous = node;
	list->head->next = node;
}

//add node after the tail
void list_add_last(node_t *node, linked_list_t *list)
{
	list->count++;
	node->next = list->tail;
	node->previous = list->tail->previous;
	list->tail->previous->next = node;
	list->tail->previous = node;
}

//remove node from the list
void list_remove_node(node_t *node, linked_list_t *list)
{
	assert(node != list->head);
	assert(node != list->tail);
	list->count--;
	node->previous->next = node->next;
	node->next->previous = node->previous;
	free(node->addr);
	free(node);
}

node_t* list_get_node_by_index(int index, linked_list_t *list)
{
	assert(index < list->count);
	node_t *i = list->head;
	for (int j = 0; j <= index; j++){
		i = i->next;
	}
	return i;
}

void list_print(linked_list_t *list)
{
	int index = 0;
	printf("########################## Players #######################\n");
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		memset(hbuf, 0, NI_MAXHOST);
		memset(sbuf, 0, NI_MAXSERV);

		getnameinfo((struct sockaddr *)i->addr, sizeof(struct sockaddr), hbuf,
			sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);

		printf("ID: %d ADDR: %s PORT: %s\n", index, hbuf, sbuf);
		index++;
	}
	printf("############################################################\n");
}

void list_reverse_print(linked_list_t *list)
{
	printf("########################## Aphorisms #######################\n");
	for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
	}
	printf("############################################################\n");
}



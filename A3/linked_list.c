/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 linked_list.c               #
###############################################
*/

#include "linked_list.h"

//set-up the head and tail of the list
void list_init(linked_list_t *list)
{
	list->count = 0;
	list->head = (node_t *)malloc(sizeof(node_t));
	list->tail = (node_t *)malloc(sizeof(node_t));
	list->tail->client = NULL;
	list->head->client = NULL;
	list->tail->buffer = NULL;
	list->head->buffer = NULL;
	list->head->next = list->tail;
	list->head->previous = NULL;
	list->tail->next = NULL;
	list->tail->previous = list->head;
}

//free the list
void list_delete(linked_list_t *list)
{
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		node_t *previous = i->previous;
		list_remove_node(i, list);
		i = previous;
	}
	free(list->head);
	free(list->tail);
}

//build a new list node
node_t *list_create_node(int fd, char *name)
{
	node_t *node = (node_t *)malloc(sizeof(node_t));

	node->client = (client_t *)malloc(sizeof(client_t));
	int name_length = strnlen(name, MAX_NICKNAME_LENGTH);
	node->client->name = (char *)malloc((name_length + 1) * sizeof(char));
	memset(node->client->name, '\0', sizeof(char) * (name_length + 1));
	strncpy(node->client->name, name, name_length);
	node->client->fd = fd;
	node->buffer = (recv_buffer_t *)malloc(sizeof(recv_buffer_t));
	node->buffer->buffer = NULL;
	node->buffer->message_length = -1;
	node->buffer->received_bytes = 0;
	return node;
}

//change the name of the client
void list_set_name_by_node(node_t *client, char *name, linked_list_t *list)
{
	assert(client != list->head);
	assert(client != list->tail);
	if (client->client->name != NULL){
		free(client->client->name);
	}
	int name_length = strnlen(name, MAX_NICKNAME_LENGTH);
	client->client->name = (char*) malloc(sizeof(char) * (MAX_NICKNAME_LENGTH + 1));
	memset(client->client->name, '\0', sizeof(char) * (MAX_NICKNAME_LENGTH + 1));
	memcpy(client->client->name, name, sizeof(char) * name_length);
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
	recv_buffer_free(node->buffer);
	client_free(node->client);
	free(node);
}

void list_print(linked_list_t *list){
	printf("########################## Clients #######################\n");
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		printf("%d: %s\n", i->client->fd, i->client->name);
	}
	printf("##########################################################\n");
}

void list_reverse_print(linked_list_t *list){
	printf("########################## Clients #######################\n");
	for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
		printf("%d: %s\n", i->client->fd, i->client->name);
	}
	printf("##########################################################\n");
}

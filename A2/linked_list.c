#include "linked_list.h"

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

void list_set_name_by_fd(int fd, char *name, linked_list_t *list)
{
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		if (i->client->fd == fd){
			list_set_name_by_node(i, name, list);
			break;
		}
	}
}

void list_set_name_by_node(node_t *client, char *name, linked_list_t *list)
{
	assert(client != list->head);
	assert(client != list->tail);
	if (client->client->name != NULL){
		free(client->client->name);
	}
	int name_length = strnlen(name, MAX_NICKNAME_LENGTH);
	client->client->name = (char*) malloc(sizeof(char) * (name_length+ 1));
	memset(client->client->name, '\0', sizeof(char) * (name_length + 1));
	memcpy(client->client->name, name, sizeof(char) * name_length);
}

void list_add_first(node_t *node, linked_list_t *list)
{
	list->count++;
	node->next = list->head->next;
	node->previous = list->head;
	list->head->next->previous = node;
	list->head->next = node;
}

void list_add_last(node_t *node, linked_list_t *list)
{
	list->count++;
	node->next = list->tail;
	node->previous = list->tail->previous;
	list->tail->previous->next = node;
	list->tail->previous = node;
}

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

void list_remove_by_fd(int fd, linked_list_t *list)
{
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		if (i->client->fd == fd){
			list_remove_node(i, list);
			break;
		}
	}
}

void list_remove_by_name(char *name, linked_list_t *list)
{
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		if (!strcmp(i->client->name, name)){
			list_remove_node(i, list);
			break;
		}
	}
}

void list_print(linked_list_t *list){
	printf("########################## Clients #######################\n");
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		printf("%d: %s\n", i->client->fd, i->client->name);
	}
}

void list_reverse_print(linked_list_t *list){
	printf("########################## Clients #######################\n");
	for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
		printf("%d: %s\n", i->client->fd, i->client->name);
	}
}

node_t *list_get_node_by_fd(int fd, linked_list_t *list)
{
	for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
		if(i->client->fd == fd){
			return i;
		}
	}
	return NULL;
}

node_t *list_get_node_by_name(char *name, linked_list_t *list)
{
	for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
		if(!strcmp(i->client->name, name)){
			return i;
		}
	}
	return NULL;
}

void recv_buffer_free(recv_buffer_t *buffer)
{
	if (buffer != NULL){
		if (buffer->buffer != NULL){
			free(buffer->buffer);
		}
		free(buffer);
	}
}

void client_free(client_t *client)
{
	if (client != NULL){
		if (client->name != NULL){
			free(client->name);
		}
		free(client);
	}
}

void recv_buffer_reset(recv_buffer_t *buffer)
{
	if(buffer != NULL){
		buffer->message_length = -1;
		buffer->message_type = -1;
		buffer->received_bytes = 0;
		free(buffer->buffer);
		buffer->buffer = NULL;
	}
}
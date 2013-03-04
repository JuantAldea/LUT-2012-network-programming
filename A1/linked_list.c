#include "linked_list.h"

void list_init(linked_list *list)
{
	list->count = 0;
	list->head = (node *)malloc(sizeof(node));
	list->tail = (node *)malloc(sizeof(node));
	list->tail->fd = -1;
	list->tail->name = NULL;
	list->head->name = NULL;
	list->head->next = list->tail;
	list->tail->next = NULL;
	list->head->previous = NULL;
	list->tail->previous = list->head;
}

void list_delete(linked_list *list)
{
	for(node *i = list->head->next; i != list->tail; i = i->next){
		list_remove_node(i, list);
	}
	free(list->head);
	free(list->tail);
}

node *list_create_node(int fd, char *name)
{
	node * n = (node*)malloc(sizeof(node));
	int name_length = strnlen(name, 30);
	n->name = (char *)malloc((name_length + 1) * sizeof(char));
	memset(n->name, '\0', sizeof(char) * (name_length + 1));
	strncpy(n->name, name, name_length);
	n->fd = fd;
	return n;
}

void list_set_name(int fd, char *name, linked_list *list)
{
	for(node *i = list->head->next; i != list->tail; i = i->next){
		if (i->fd == fd){
			if (i->name != NULL){
				free(i->name);
			}
			int name_length = strnlen(name, 30);
			i->name = (char*) malloc(sizeof(char) * (name_length+ 1));
			memset(i->name, '\0', sizeof(char) * (name_length + 1));
			memcpy(i->name, name, sizeof(char) * name_length);
			break;
		}
	}
}

void list_add_first(node *n, linked_list *list)
{
	list->count++;
	n->next = list->head->next;
	n->previous = list->head;
	list->head->next->previous = n;
	list->head->next = n;
}

void list_add_last(node *n, linked_list *list)
{
	list->count++;
	n->next = list->tail;
	n->previous = list->tail->previous;
	list->tail->previous->next = n;
	list->tail->previous = n;
}

void list_remove_node(node *n, linked_list *list)
{
	assert(n != list->head);
	assert(n != list->tail);
	list->count--;
	n->previous->next = n->next;
	n->next->previous = n->previous;
	free(n->name);
	free(n);
}

void list_remove_by_fd(int fd, linked_list *list)
{
	for(node *i = list->head->next; i != list->tail; i = i->next){
		if (i->fd == fd){
			list_remove_node(i, list);
			break;
		}
	}
}

void list_remove_by_name(char *name, linked_list *list)
{
	for(node *i = list->head->next; i != list->tail; i = i->next){
		if (!strcmp(i->name, name)){
			list_remove_node(i, list);
			break;
		}
	}
}

void list_print(linked_list *list){
	printf("########################## Clients #######################\n");
	for(node *i = list->head->next; i != list->tail; i = i->next){
		printf("%d: %s\n", i->fd, i->name);
	}
}

void list_reverse_print(linked_list *list){
	printf("########################## Clients #######################\n");
	for(node *i = list->tail->previous; i != list->head; i = i->previous){
		printf("%d: %s\n", i->fd, i->name);
	}
}
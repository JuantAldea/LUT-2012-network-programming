#include "user_node.h"
#define MAX_NAME_LENGTH 15

user_node_init(user_node **node)
{
	return ((*node = (user_node *) malloc(sizeof(user_node))==NULL ? 1 : 0);
}

int user_node_free(user_node *node)
{
	return free(node);
}

int user_node_fill(char *name, int fd, user_node *node)
{
	int error = 0;
	if(node == NULL){
		error = user_node_init(&node);
	}
	node->fd = fd;
	return user_node_set_name(name, node);
}

user_node_set_name(char *name, user_node *node)
{
	int name_length = strnlen(name, MAX_NAME_LENGTH);
	if (node->name != NULL){
		free(node->name);
	}
	node->name = (char*) malloc(sizeof(char) * (name_length+ 1));
	memset(node->name, '\0', sizeof(char) * (name_length + 1));
	memcpy(i->name, name, sizeof(char) * name_length);
}

user_node_compare_id(user_node *a, user_node *b)
{
	return a->fd == b->fd;
}

user_node_compare_name(user_node *a, user_node *b)
{
	return (strncmp(a->name, b->name, MAX_NAME_LENGTH)==0) ? 1 : 0
}

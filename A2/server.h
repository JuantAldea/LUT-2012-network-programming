#ifndef __SERVER_H__
#define __SERVER_H__

#include "common.h"
#include "server_commands.h"
#include "linked_list.h"

#define BACKLOG 100

int server(char *port);

int prepare_server (char *port);

int parse_command(char *command);

node *manage_disconnect(int fd, linked_list *users);

void disconnect_clients(linked_list *list);

int manage_nick_change_by_fd(int fd, char *name, linked_list *users);

int manage_nick_change_by_node(node *i, char *name, linked_list *users);

int nick_chage_by_fd(int fd, char *name, linked_list *users);

int nick_change_by_node(node *i, char *name, linked_list *users);

#endif

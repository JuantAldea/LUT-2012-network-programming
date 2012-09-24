#ifndef __SERVER_H__
#define __SERVER_H__

#include "common.h"
#include "server_commands.h"
#include "linked_list.h"

#define BACKLOG 100

int server(char *port);

int prepare_server (char *port);

int parse_command(char *command);

void manage_disconnect_by_node(node_t *user, linked_list_t *users, int polite);

void disconnect_clients(linked_list_t *list);

int manage_nick_change_by_fd(int fd, char *name, linked_list_t *users);

int manage_nick_change_by_node(node_t *i, char *name, linked_list_t *users);

int nick_chage_by_fd(int fd, char *name, linked_list_t *users);

int nick_change_by_node(node_t *i, char *name, linked_list_t *users);

void split_connect_message(recv_buffer_t *buffer, char **nickname, char **introduction);

#endif

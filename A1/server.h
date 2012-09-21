#ifndef __SERVER_H__
#define __SERVER_H__

#include "common.h"
#include "server_commands.h"
#include "linked_list.h"

#define BACKLOG 100

int server(char *port);

int prepare_server (char *address, char *port);

int parse_command(char *command);

void disconnect_clients(linked_list *list);

#endif

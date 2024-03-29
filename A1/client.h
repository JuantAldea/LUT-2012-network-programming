#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "common.h"
#include "client_commands.h"

int client(char *name, char *address, char *port);
int prepare_connection(char *address, char *port);
int client_parse_command(char *command);
#endif
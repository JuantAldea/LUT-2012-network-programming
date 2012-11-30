#ifndef __TCP_H__
#define __TCP_H__

#include "system_headers.h"

#define BACKLOG 100

int prepare_server_TCP (char *port);
int prepare_connection_TCP(char *address, char *port);

#endif

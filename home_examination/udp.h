#ifndef __UDP_H__
#define __UDP_H__

#include "system_headers.h"

int prepare_server_UDP(char *port, short family);
int prepare_client_UDP(char *address, char *port, struct sockaddr *server, socklen_t *addrlen);

#endif

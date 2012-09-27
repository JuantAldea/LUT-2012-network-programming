#ifndef __CLIENT_T_H__
#define __CLIENT_T_H__

#include <stdlib.h>

typedef struct client_s client_t;
struct client_s{
	int fd;
	char *name;
};

void client_free(client_t *client);

#endif

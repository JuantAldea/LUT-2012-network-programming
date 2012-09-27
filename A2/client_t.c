#include "client_t.h"

void client_free(client_t *client)
{
	if (client != NULL){
		if (client->name != NULL){
			free(client->name);
		}
		free(client);
	}
}

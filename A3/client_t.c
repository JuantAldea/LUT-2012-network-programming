/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 client_t.c                  #
###############################################
*/
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

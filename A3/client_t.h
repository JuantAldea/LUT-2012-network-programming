/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  client_t.h                 #
###############################################
*/

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

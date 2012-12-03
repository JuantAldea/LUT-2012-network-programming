/*
###############################################
#        CT30A5001 - Network Programming      #
#               Home Examination              #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################
*/

#ifndef __TCP_H__
#define __TCP_H__

#include "system_headers.h"

#define BACKLOG 100

int prepare_server_TCP(char *port, short family);
int prepare_connection_TCP(char *address, char *port);

#endif

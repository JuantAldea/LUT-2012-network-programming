/*
* CT30A5001 Network Programming
* sctps.h, STCP server and client example
*
* Contains header inclusions and function prototypes for SCTP server.
*
* Author:
*   Jussi Laakkonen
*   1234567
*   jussi.laakkonen@lut.fi
*/

#ifndef __SCTP_TESTSERVER_H_
#define __SCTP_TESTSERVER_H_

// Length for message buffer
#define MBUFFERLEN 1024*6

#define ONLYRUNNING 1
#define NOLOOPBACK 0

#include "addresses.h"
#include "game.h"
#include "linked_list.h"
#include "protocol.h"

int run_test_server(int argc, char* argv[]);
void sighandler(int);
int testserver_input_error(char*);

int broadcast_start(int sctp_socket, int8_t columns, int8_t rows, linked_list_t *list);
#endif

/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              sctp.h               #
####################################################
*/

#ifndef __SCTP_CLIENT_H_
#define __SCTP_CLIENT_H_

#include <time.h>

#include "sctp_utils.h"
#include "protocol.h"
#include "client_command_line.h"
#include "game.h"
#include <sys/ioctl.h>

int client(int argc, char* argv[]);
int testclient_input_error(char*);
char *testclient_fill_random_data(int);
void print_addr_type(char*);
int check_addr_type(char*, int);
void sighandler(int sig);
#endif

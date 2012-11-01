/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              client_command_line.c               #
####################################################
*/

#ifndef __CLIENT_COMMAND_LINE_H__
#define __CLIENT_COMMAND_LINE_H__

#include <string.h>
#include <stdlib.h>

#define JOIN_COMMAND_STR    "/join"
#define READY_COMMAND_STR   "/ready"
#define AREA_COMMAND_STR  "/area"
#define PLACE_COMMAND_STR   "/place"
#define QUIT_COMMAND_STR    "/quit"

#define JOIN_COMMAND_CODE 0
#define READY_COMMAND_CODE  1
#define AREA_COMMAND_CODE   2
#define PLACE_COMMAND_CODE   3
#define QUIT_COMMAND_CODE  4
#define UNKOWN_COMMAND_CODE     255

int parse_command(char *command, int params[]);

#endif
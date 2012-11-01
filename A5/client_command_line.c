/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              client_command_line.c               #
####################################################
*/

#include "client_command_line.h"

int parse_command(char *command, int params[])
{
    if (!strcmp(command, JOIN_COMMAND_STR)){
        return JOIN_COMMAND_CODE;
    }else if(!strcmp(command, READY_COMMAND_STR)){
        return READY_COMMAND_CODE;
    }else if(!strcmp(command, AREA_COMMAND_STR)){
        return AREA_COMMAND_CODE;
    }else if(!strncmp(command, PLACE_COMMAND_STR, 6)){
        params[0] = atoi(&command[7]);
        return PLACE_COMMAND_CODE;
    }else if(!strcmp(command, QUIT_COMMAND_STR)){
        return QUIT_COMMAND_CODE;
    }
    return UNKOWN_COMMAND_CODE;
}

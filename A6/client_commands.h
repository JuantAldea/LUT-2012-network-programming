/*
###############################################
#        CT30A5001 - Network Programming      #
#           Assignment 6: FTP client          #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#              client_commands.h              #
###############################################
*/
#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#define OPEN_COMMAND    "open"
#define CD_COMMAND      "cd"
#define CLOSE_COMMAND   "close"
#define ACTIVE_COMMAND  "active"
#define PASSIVE_COMMAND "passive"
#define GET_COMMAND     "get"
#define PUT_COMMAND     "put"
#define LS_COMMAND      "ls"
#define BINARY_COMMAND  "binary"
#define QUIT_COMMAND    "quit"
#define HELP_COMMAND    "help"
#define SYST_COMMAND    "syst"
#define STATUS_COMMAND  "status"
#define RAW_COMMAND     "raw"


#define OPEN_COMMAND_CODE    0
#define CD_COMMAND_CODE      1
#define CLOSE_COMMAND_CODE   2
#define ACTIVE_COMMAND_CODE  3
#define PASSIVE_COMMAND_CODE 4
#define GET_COMMAND_CODE     5
#define PUT_COMMAND_CODE     6
#define LS_COMMAND_CODE      7
#define BINARY_COMMAND_CODE  8
#define QUIT_COMMAND_CODE    9
#define HELP_COMMAND_CODE    10
#define STATUS_COMMAND_CODE  11
#define SYST_COMMAND_CODE    12
#define RAW_COMMAND_CODE     13

#define EOF_COMMAND_CODE     254
#define UNKOWN_COMMAND_CODE  255

#endif
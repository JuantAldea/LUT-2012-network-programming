/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 sever_commands.h            #
###############################################
*/

#ifndef __SERVER_COMMANDS_H__
#define __SERVER_COMMANDS_H__

#define LIST_COMMAND_STR 		"/list"
#define STOP_COMMAND_STR		"/stop"
#define START_COMMAND_STR		"/start"
#define SHUTDOWN_COMMAND_STR	"/quit"

#define SHUTDOWN_COMMAND_CODE	0
#define LIST_COMMAND_CODE		1
#define STOP_COMMAND_CODE 		2
#define START_COMMAND_CODE		3
#define UNKOWN_COMMAND_CODE 	255

#endif

/*
###############################################
#		CT30A5001 - Network Programming		  #
#		Assignment2: TCP multiuser chat		  #
# 	Juan Antonio Aldea Armenteros (0404450)   #
# 		juan.aldea.armenteros@lut.fi		  #
#				client_commands.c			  #
###############################################
*/
#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#define NICK_COMMAND 	"nick"
#define CONNECT_COMMAND "connect"
#define WHO_COMMAND 	"who"
#define QUIT_COMMAND 	"quit"

#define NICK_COMMAND_CODE  		0
#define CONNECT_COMMAND_CODE  	1
#define WHO_COMMAND_CODE  		2
#define QUIT_COMMAND_CODE		3
#define CHAT_COMMAND_CODE		4
#define UNKOWN_COMMAND_CODE 	255

#endif
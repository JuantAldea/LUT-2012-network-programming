#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#define MAX_COMMAND_LENGTH 10
#define COMMAND_LENGTH_BUFFER 11

#define NICK_COMMAND 	"/nick"
#define CONNECT_COMMAND "/connect"
#define WHO_COMMAND 	"/who"
#define QUIT_COMMAND 	"/quit"

#define NICK_COMMAND_CODE  		0
#define CONNECT_COMMAND_CODE  	1
#define WHO_COMMAND_CODE  		2
#define QUIT_COMMAND_CODE		3
#define UNKOWN_COMMAND_CODE 	255

#endif
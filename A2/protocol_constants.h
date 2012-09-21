#ifndef __PROTOCOL_CONSTANTS_H__
#define __PROTOCOL_CONSTANTS_H__

#define PROTOCOL_HEADER_SIZE 3

#define CONNECT_MSG 	0
#define TEXT_MSG     	1
#define WHO_MSG     	2
#define QUIT_MSG    	3
#define HELLO_FWD_MSG	4
#define NICK_MSG    	5
#define MSG_FWD_MSG 	6
#define CLIENT_LIST_MSG 7
#define CLIENT_LEFT_MSG 8
#define ERROR_MSG 		9
#define UNKOWN_MSG		255

#endif

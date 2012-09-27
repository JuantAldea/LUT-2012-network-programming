/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                protocol_constant.h          #
###############################################
*/

#ifndef __PROTOCOL_CONSTANTS_H__
#define __PROTOCOL_CONSTANTS_H__

#define PROTOCOL_HEADER_SIZE 3

#define CONNECT_MSG 	0
#define CHAT_MSG     	1
#define WHO_REQUEST_MSG 2
#define QUIT_MSG    	3
#define INTRODUCTION_FWD_MSG	4
#define ACCEPT_NICKNAME_MSG    	5
#define CHAT_FWD_MSG 	6
#define CLIENT_LIST_MSG 7
#define CLIENT_LEFT_MSG 8
#define ERROR_MSG 		9
#define UNKOWN_MSG		255

#endif

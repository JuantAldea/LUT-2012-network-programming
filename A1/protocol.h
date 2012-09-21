#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "protocol_constants.h"
#include "common.h"


int send_msg(int socket, uchar *msg, uint8_t type);
int recv_msg(int socket, uchar **msg, uint8_t *type);

//client side
int send_login(int socket, uchar *username);
int send_disconnect(int socket, double delay);

//server side
int send_clock(int socket);
int send_news_item(int socket, uchar *news_item);
int send_error(int socket, uchar *msg);

//int parse_msg(char *msg);

//both?


#endif
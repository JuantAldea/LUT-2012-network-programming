#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stropts.h>
#include <sys/ioctl.h>
#include "protocol_constants.h"
#include "common.h"
#include "recv_buffer.h"
#include "linked_list.h"

#define MAX_NICKNAME_LENGTH 15
#define HEADER_SIZE 6

//base
int send_msg(int socket, uchar *msg, int msg_size, int16_t type);
int recv_msg(int socket, recv_buffer_t *buffer, int *full_message);

//client to server
int send_login(int socket, char *username, char *introduction_message);
int send_chat(int socket, char *msg);
int send_who_request(int socket);
int send_disconnect(int socket, char *quit_message);

//server to client
int send_fwd_client_left(int socket, char *nickname, char *quit_message);
int send_user_list(int socket, linked_list_t *users);
int send_fwd_chat_msg(int socket, char *nickname, char *msg);
int send_accept_nickname_change(int socket, char *nickname);
int send_fwd_introduction_msg(int socket, char *nickname, char *msg);
int send_error(int socket, char *msg);
void buffer_dump(char * buffer, int length);

#endif

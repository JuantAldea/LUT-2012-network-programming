#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stropts.h>
#include <sys/ioctl.h>
#include "protocol_constants.h"
#include "common.h"
#include "recv_buffer.h"
#include "linked_list.h"

int send_msg(int socket, uchar *msg, int16_t type);

int recv_msg(int socket, recv_buffer_t *buffer, int *full_message);





int send_login(int socket, char *username, char *introduction_message);
int send_chat(int socket, char *msg);
int send_who_request(int socket);
int send_disconnect(int socket, char *quit_message);


int send_error(int socket, char *msg);

#endif

#ifndef __CHAT_PROTOCOL_H__
#define __CHAT_PROTOCOL_H__

#include <inttypes.h>
#include "linked_list.h"
#include "system_headers.h"

void chat_forward_msg(uint8_t sender_id, char msg[129], linked_list_t *clients);
int chat_recv(int socket, char msg[131], int *fullmsg);
int chat_send(int socket, uint8_t sender_id, char msg[129]);

#endif

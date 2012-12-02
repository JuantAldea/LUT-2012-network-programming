#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#include "system_headers.h"
#include "linked_list.h"
#include "chat_protocol.h"
#include <inttypes.h>

extern linked_list_t *player_list;

void chat_server(int socket);
void chat_forward_msg(uint8_t sender_id, char msg[129], linked_list_t *clients);

#endif

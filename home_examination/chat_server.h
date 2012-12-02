#ifndef __CHAT_SERVER_H__
#define __CHAT_SERVER_H__

#include "system_headers.h"
#include "linked_list.h"
#include "chat_protocol.h"
#include <inttypes.h>

extern linked_list_t *player_list;

void chat_server(int socket);

#endif

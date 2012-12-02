#ifndef __SERVER_H__
#define __SERVER_H__

#include "system_headers.h"
#include "linked_list.h"
#include "map.h"
#include "tcp.h"
#include "udp.h"
#include "map_server.h"
#include "game_server.h"
#include "chat_server.h"
#include "game_protocol.h"
#include "chat_protocol.h"

void kick_players(linked_list_t *player_list);

#endif

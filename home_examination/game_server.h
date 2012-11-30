#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

#include "system_headers.h"
#include "linked_list.h"
#include "map.h"

#define CONNECT 0
#define GAME_INFO 1
#define READY 3
#define SPAWN 4

#define HEALTH_POINTS 10
void game_server(int socket);
int send_game_info(int socket, player_info_t *player);

#endif

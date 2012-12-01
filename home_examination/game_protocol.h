#ifndef __GAME_PROTOCOL_H__
#define __GAME_PROTOCOL_H__

#include "system_headers.h"
#include "linked_list.h"
#include "map.h"
#define CONNECT 0
#define GAME_INFO 1
#define READY 3
#define SPAWN 4

int send_connect(int socket, struct sockaddr *addr, socklen_t address_len);
int send_game_info(int socket, map_t *map,  player_info_t *player);
int send_ready(int socket, struct sockaddr *addr, socklen_t address_len);
int send_spawn(int socket, player_info_t *player);

#endif


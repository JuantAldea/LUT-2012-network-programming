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

void print_mapcycle(linked_list_t *list);
void parse_mapcycle(linked_list_t **list);
void delete_mapcycle_list(linked_list_t **list);
void game_server_init();
void game_server_shutdown();
#endif

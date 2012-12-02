#ifndef __GAME_SERVER_H__
#define __GAME_SERVER_H__

#include "system_headers.h"
#include "linked_list.h"
#include "map.h"
#include "game_protocol.h"
#include "chat_protocol.h"
#include <ncurses.h>

#define HEALTH_POINTS 10
void game_server(int socket);

void print_mapcycle(linked_list_t *list);
void parse_mapcycle(linked_list_t **list);
void delete_mapcycle_list(linked_list_t **list);
void game_server_init();
void game_server_shutdown();
void remove_idle_players();

uint8_t get_first_free_id();

int is_position_a_wall(map_t *map, int posx, int posy);

void respawn_death_players(int socket);
void respawn_player(player_info_t *player_info);

#endif

#ifndef __GAME_PROTOCOL_H__
#define __GAME_PROTOCOL_H__

#include "system_headers.h"
#include "linked_list.h"
#include "map.h"

#define CONNECT 0
#define GAME_INFO 1
#define READY 3
#define SPAWN 4
#define MOVE 5

#define MOVE_ACK 6
#define HIT_ACK 7
#define WALL_ACK 8
#define SUICIDE_ACK 9
#define KILL_ACK 10
#define KILLED_ACK 11

int send_connect(int socket, struct sockaddr *addr, socklen_t address_len);
int send_game_info(int socket, map_t *map,  player_info_t *player);
int send_ready(int socket, struct sockaddr *addr, socklen_t address_len);
int send_spawn(int socket, player_info_t *player);
int send_move(int socket, uint16_t keycode, struct sockaddr *addr, socklen_t address_len);
int send_move_ack(int socket, player_info_t *player_updated, player_info_t *player_to_notify);
int send_hit_ack(int socket, player_info_t *player);
int send_wall_ack(int socket, player_info_t *player);
int send_suicide_ack(int socket, player_info_t *player);

int send_kill_ack(int socket, uint8_t playerid, player_info_t *player);
int send_killed_ack(int socket, uint8_t playerid, player_info_t *player);

void broadcast_move_ack(int socket, player_info_t *player_updated, linked_list_t *players);
void send_positions_refresh(int socket, player_info_t *player_to_refresh, linked_list_t *players);
#endif

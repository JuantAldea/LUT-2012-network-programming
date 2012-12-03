#include "game_protocol.h"

int send_game_info(int socket, map_t *map,  player_info_t *player)
{
    char buffer[43];
    memset(buffer, 0, 43);
    buffer[0] = GAME_INFO;
    buffer[1] = player->playerID;
    buffer[2] = player->current_health;
    memcpy(&buffer[3], map->map_id, 8);
    memcpy(&buffer[11], map->hash, 32);
    return sendto(socket, buffer, 43, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_spawn(int socket, player_info_t *player_updated, player_info_t *player_to_notify)
{
    char buffer[3];
    buffer[0] = SPAWN;
    buffer[1] = player_updated->playerID;
    buffer[2] = player_updated->position[0];
    buffer[3] = player_updated->position[1];
    return sendto(socket, buffer, 4, 0, (struct sockaddr*)&player_to_notify->addr, player_to_notify->addr_len);
}

int send_connect(int socket, uint8_t mode, struct sockaddr *addr, socklen_t address_len)
{
    uint8_t buffer[2];
    buffer[0] = CONNECT;
    buffer[1] = mode;
    return sendto(socket, buffer, 2, 0, addr, address_len);
}

int send_ready(int socket, struct sockaddr *addr, socklen_t address_len)
{
    char buffer = READY;
    return sendto(socket, &buffer, 1, 0, addr, address_len);
}

int send_move(int socket, uint16_t keycode, struct sockaddr *addr, socklen_t address_len)
{
    int size = sizeof(uint16_t) + sizeof(uint8_t);
    uint8_t buffer[size];
    buffer[0] = MOVE;
    *(uint16_t*)&buffer[1] = htons(keycode);
    return sendto(socket, buffer, size, 0, addr, address_len);
}

int send_move_ack(int socket, player_info_t *player_updated, player_info_t *player_to_notify)
{
    uint8_t buffer[3];
    buffer[0] = MOVE_ACK;
    buffer[1] = player_updated->playerID;
    buffer[2] = player_updated->position[0];
    buffer[3] = player_updated->position[1];
    return sendto(socket, &buffer, 4, 0, (struct sockaddr*)&player_to_notify->addr, player_to_notify->addr_len);
}

int send_hit_ack(int socket, player_info_t *player)
{
    uint8_t buffer[2];
    buffer[0] = HIT_ACK;
    buffer[1] = player->current_health;
    return sendto(socket, &buffer, 2, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_wall_ack(int socket, player_info_t *player)
{
    uint8_t buffer[2];
    buffer[0] = WALL_ACK;
    buffer[1] = player->current_health;
    return sendto(socket, &buffer, 2, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_suicide_ack(int socket, player_info_t *player)
{
    uint8_t buffer[3];
    buffer[0] = SUICIDE_ACK;
    buffer[1] = player->frags;
    buffer[2] = player->deaths;
    return sendto(socket, &buffer, 3, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_kill_ack(int socket, uint8_t killed_id, player_info_t *player)
{
    uint8_t buffer[6];
    buffer[0] = KILL_ACK;
    buffer[1] = killed_id;
    buffer[2] = player->frags;
    return sendto(socket, &buffer, 3, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_killed_ack(int socket, uint8_t killer_id, player_info_t *player)
{
    uint8_t buffer[2];
    buffer[0] = KILLED_ACK;
    buffer[1] = killer_id;
    return sendto(socket, &buffer, 2, 0, (struct sockaddr*)&player->addr, player->addr_len);
}


void broadcast_move_ack(int socket, player_info_t *player_updated, linked_list_t *players)
{

    for (node_t *i = players->head->next; i != players->tail; i = i->next){
        player_info_t *player_to_update = (player_info_t*)i->data;
        if(send_move_ack(socket, player_updated, player_to_update) <= 0){
            printf("Error Broadcasting move ack: %s\n", strerror(errno));
        }
    }
}

void send_positions_refresh(int socket, player_info_t *player_to_refresh, linked_list_t *players)
{
    for (node_t *i = players->head->next; i != players->tail; i = i->next){
        player_info_t *player = (player_info_t*)i->data;
        if (player->playerID == 255) {continue;}
        if (player != player_to_refresh){
            if(send_move_ack(socket, player, player_to_refresh) <= 0){
                printf("Error Broadcasting move ack: %s\n", strerror(errno));
            }
        }
    }
}

void broadcast_disconnection_ack(int socket, uint8_t playerid, linked_list_t *players)
{
    if (playerid == 255) {return;}
    for (node_t *i = players->head->next; i != players->tail; i = i->next){
        player_info_t *player_to_update = (player_info_t*)i->data;
        if(send_disconnection_ack(socket, playerid, player_to_update) <= 0){
            printf("Error Broadcasting disconnect ack: %s\n", strerror(errno));
        }
    }
}

int send_disconnection_ack(int socket, uint8_t playerid, player_info_t *player)
{
    uint8_t buffer[2];
    buffer[0] = DISCONNECT_ACK;
    buffer[1] = playerid;
    return sendto(socket, buffer, 2, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

void broadcast_death_ack(int socket, uint8_t playerid, linked_list_t *players)
{
    if (playerid == 255){return;}
    for (node_t *i = players->head->next; i != players->tail; i = i->next){
        player_info_t *player_to_update = (player_info_t*)i->data;
        if(send_death_ack(socket, playerid, player_to_update) <= 0){
            printf("Error Broadcasting death ack: %s\n", strerror(errno));
        }
    }
}


int send_death_ack(int socket, uint8_t death_id, player_info_t *player)
{
    uint8_t buffer[2];
    buffer[0] = DEATH_ACK;
    buffer[1] = death_id;
    return sendto(socket, &buffer, 2, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

void broadcast_spawn(int socket, player_info_t *updated_player, linked_list_t *players)
{
    if (updated_player->playerID == 255){return;}
    for (node_t *i = players->head->next; i != players->tail; i = i->next){
        player_info_t *player_to_update = (player_info_t*)i->data;
        if(send_spawn(socket, updated_player, player_to_update) <= 0){
            printf("Error Broadcasting spawn ack: %s\n", strerror(errno));
        }
    }
}

int send_ping(int socket, struct sockaddr *addr, socklen_t address_len)
{
    uint8_t buffer[2];
    buffer[0] = PING;
    buffer[1] = rand() % 256;
    int bytes = sendto(socket, &buffer, 2, 0, addr, address_len);
    return bytes <= 0 ? bytes : buffer[1];
}

int send_pong(int socket, uint8_t ping_id, player_info_t *player)
{
    uint8_t buffer[2];
    buffer[0] = PONG;
    buffer[1] = ping_id;
    return sendto(socket, &buffer, 2, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_map_change(int socket, player_info_t *player)
{
    uint8_t buffer = MAP_CHANGE;
    return sendto(socket, &buffer, 1, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

void broadcast_map_change(int socket, linked_list_t *players)
{
    for (node_t *i = players->head->next; i != players->tail; i = i->next){
        player_info_t *player = (player_info_t*)i->data;
        if (send_map_change(socket, player) <= 0){
            printf("Error Broadcasting map change: %s\n", strerror(errno));
        }
    }
}


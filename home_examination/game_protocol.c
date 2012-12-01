#include "game_protocol.h"

int send_game_info(int socket, map_t *map,  player_info_t *player)
{
    char buffer[43];
    memset(buffer, 0, 43);
    buffer[0] = GAME_INFO;
    buffer[1] = player->playerID;
    buffer[2] = player->current_health;
    memcpy(&buffer[3], map->map_id, 8);
    printf("%.*s\n", map->map_id);
    memcpy(&buffer[11], map->hash, 32);
    printf("SENDGAMEINFO%.*s", 32, map->hash);
    return sendto(socket, buffer, 43, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_spawn(int socket, player_info_t *player)
{
    char buffer[3];
    buffer[0] = SPAWN;
    buffer[1] = player->position[0];
    buffer[2] = player->position[1];
    return sendto(socket, buffer, 3, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

int send_connect(int socket, struct sockaddr *addr, socklen_t address_len)
{
    char buffer = CONNECT;
    return sendto(socket, &buffer, 1, 0, addr, address_len);
}

int send_ready(int socket, struct sockaddr *addr, socklen_t address_len)
{
    char buffer = READY;
    return sendto(socket, &buffer, 1, 0, addr, address_len);
}

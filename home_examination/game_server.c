#include "game_server.h"

extern linked_list_t *player_list;
extern char current_map_id[];

map_t *current_map;

void game_server(int socket)
{
    struct sockaddr_storage sender_address;
    memset(&sender_address, 0, sizeof(struct sockaddr_storage));
    socklen_t sender_address_size = sizeof(struct sockaddr_storage);
    uchar recvbuffer[1024];
    memset(recvbuffer, 0, 1024);

    if (recvfrom(socket, &recvbuffer, 1024, 0, (struct sockaddr*)&sender_address, &sender_address_size) < 0){
        perror("Recvfrom multicast");
    }
    printf("Game Server\n");
    switch (recvbuffer[0]){
        case CONNECT:
        {

        }
        break;
        case READY:
        default:
        {
            node_t *player_node = list_search_by_addrinfo(&sender_address, player_list);
            if (player_node == NULL){
                player_info_t *player_info = malloc(sizeof(player_info_t));
                player_info->addr = sender_address;
                player_info->addr_len = sender_address_size;
                player_info->chat_descriptor = -1;
                player_info->playerID = -1;
                player_info->current_health = HEALTH_POINTS;
                player_info->position[0] = 5;
                player_info->position[1] = 5;
                player_info->frags = 0;
                player_info->deaths = 0;
                memset(&player_info->last_action, 0, sizeof(struct timeval));
                player_info->last_udp_package = 0;
                send_game_info(socket, player_info);
                player_node = list_create_node(player_info);
                list_add_last(player_node, player_list);
            }
        }
        break;
    }
}

int send_game_info(int socket, player_info_t *player)
{
    char *buffer = malloc(sizeof(char) * 40  + sizeof(uint8_t) * 3);
    memset(buffer, 0, 11);
    buffer[0] = GAME_INFO;
    buffer[1] = player->playerID;
    buffer[2] = player->current_health;
    memcpy(&buffer[3], current_map_id, 8);
    return sendto(socket, buffer, 11, 0, (struct sockaddr*)&player->addr, player->addr_len);
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

int send_spawn(int socket, struct sockaddr *addr, socklen_t address_len)
{
    char buffer[3];
    buffer[0] = SPAWN;
    buffer[1] = 5;
    buffer[2] = 5;
    return sendto(socket, buffer, 1, 0, addr, address_len);
}


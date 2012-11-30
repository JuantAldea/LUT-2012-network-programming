#include "game_server.h"

extern linked_list_t *player_list;
linked_list_t *mapcycle_list = NULL;
map_t *current_map = NULL;
char *current_map_id;

void game_server_init()
{
    parse_mapcycle(&mapcycle_list);
    char path[50];
    current_map_id = mapcycle_list->head->next->data;
    sprintf(path, "serverdata/%.*s.map", 8, current_map_id);
    current_map = malloc(sizeof(map_t));
    printf("[GAME SERVER] Loading first map: %s\n", path);
    read_map(path, current_map);
}

void game_server_shutdown()
{
    free_map(current_map);
    free(current_map);
    delete_mapcycle_list(&mapcycle_list);
}

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
    //try to find the client
    node_t *player_node = list_search_by_addrinfo(&sender_address, player_list);
    printf("Game Server\n");
    switch (recvbuffer[0]){
        case CONNECT:{
            if (player_node == NULL){
                if (player_list->count < current_map->max_players){
                    player_info_t *player_info = malloc(sizeof(player_info_t));
                    player_info->addr = sender_address;
                    player_info->addr_len = sender_address_size;
                    player_info->chat_descriptor = -1;
                    player_info->playerID = get_first_free_id();
                    player_info->current_health = HEALTH_POINTS;
                    player_info->position[0] = current_map->starting_positions[player_info->playerID - 1][0];
                    player_info->position[1] = current_map->starting_positions[player_info->playerID - 1][1];
                    player_info->frags  = 0;
                    player_info->deaths = 0;
                    player_info->last_udp_package = 0;
                    gettimeofday(&player_info->last_action, NULL);
                    send_game_info(socket, player_info);
                    player_node = list_create_node(player_info);
                    list_add_last(player_node, player_list);
                }else{
                    //send error, server full
                }
            }
        }
        break;
        case READY:{
            //the client is known
            if (player_node != NULL){
                send_spawn(socket, (player_info_t*)player_node->data);
            }
        }
        default:
        break;
    }
    //update the idle time;
    if (player_node != NULL){
        gettimeofday(&((player_info_t*)player_node->data)->last_action, NULL);
    }
}

int send_game_info(int socket, player_info_t *player)
{
    char buffer[43];
    memset(buffer, 0, 43);
    buffer[0] = GAME_INFO;
    buffer[1] = player->playerID;
    buffer[2] = player->current_health;
    memcpy(&buffer[3], current_map_id, 8);
    memcpy(&buffer[11], current_map->hash, 32);
    return sendto(socket, buffer, 43, 0, (struct sockaddr*)&player->addr, player->addr_len);
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

int send_spawn(int socket, player_info_t *player)
{
    char buffer[3];
    buffer[0] = SPAWN;
    buffer[1] = player->position[0];
    buffer[2] = player->position[1];
    return sendto(socket, buffer, 3, 0, (struct sockaddr*)&player->addr, player->addr_len);
}

void parse_mapcycle(linked_list_t **list)
{
    *list = (linked_list_t*)malloc(sizeof(linked_list_t));
    list_init(*list);
    char *buffer = NULL;
    FILE *mapcycle_file = fopen("mapcycle.txt", "r");
    if (mapcycle_file == NULL){
        printf("ERROR opening the file %s\n", strerror(errno));
        return;
    }
    int readed_bytes;
    size_t readed;
    while ((readed_bytes = getline(&buffer, &readed, mapcycle_file)) > 0){
        sscanf(buffer, "%s", buffer);
        list_add_last(list_create_node(buffer), *list);
        buffer = NULL;
    }
    free(buffer);
    fclose(mapcycle_file);
}

void print_mapcycle(linked_list_t *list)
{
    printf("#############################\n");
    for (node_t *i = list->head->next; i != list->tail; i = i->next){
        printf("%s\n", (char*)i->data);
    }
    printf("#############################\n");
}

void delete_mapcycle_list(linked_list_t **list)
{
    list_delete(*list);
    free(*list);
    *list = NULL;
}

uint8_t get_first_free_id()
{
    int max_players = current_map->max_players;
    if (max_players <= player_list->count){
        return -1;
    }
    uint8_t used_ids[max_players];
    memset(used_ids, 0, max_players);

    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player = (player_info_t *)i->data;
        used_ids[player->playerID - 1] = 1;
    }

    for (uint8_t i = 0; i < max_players; i++){
        if (!used_ids[i]){
            return i + 1;
        }
    }
    return -1;
}
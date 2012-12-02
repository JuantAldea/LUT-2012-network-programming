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
                    memset(player_info, 0, sizeof(player_info_t));
                    player_info->addr = sender_address;
                    player_info->addr_len = sender_address_size;
                    player_info->chat_descriptor = -1;
                    player_info->playerID = get_first_free_id();
                    player_info->current_health = HEALTH_POINTS;
                    player_info->position[0] = current_map->starting_positions[player_info->playerID - 1][0];
                    player_info->position[1] = current_map->starting_positions[player_info->playerID - 1][1];
                    // player_info->frags  = 0;
                    // player_info->deaths = 0;
                    // player_info->last_udp_package = 0;
                    gettimeofday(&player_info->last_action, NULL);
                    send_game_info(socket, current_map, player_info);
                    player_node = list_create_node(player_info);
                    list_add_last(player_node, player_list);
                    printf("[GAME SERVER]: Player %d connected\n", player_info->playerID);
                }else{
                    //send error, server full
                    printf("[GAME SERVER]: Server full\n");
                }
            }
        }
        break;
        case READY:{
            //the client is known
            if (player_node != NULL){
                printf("[GAME SERVER]: Sending spawn to Player %d: (%d, %d)\n",
                    ((player_info_t*)player_node->data)->playerID,
                    ((player_info_t*)player_node->data)->position[0],
                    ((player_info_t*)player_node->data)->position[1]
                    );
                send_spawn(socket, (player_info_t*)player_node->data);
                //inform the new client about the other players
                send_positions_refresh(socket, (player_info_t*)player_node->data, player_list);
                //inform the old clients about the new one
                broadcast_move_ack(socket, (player_info_t*)player_node->data, player_list);
            }
        }
        break;
        case MOVE:{
           if (player_node != NULL && ((player_info_t*)player_node->data)->current_health > 0){
                player_info_t *player = (player_info_t*)player_node->data;
                uint8_t x = player->position[0];
                uint8_t y = player->position[1];
                uint16_t key = ntohs(*(uint16_t*)(recvbuffer +1));
                switch(key){
                    case KEY_UP:
                        y--;
                    break;
                    case KEY_DOWN:
                        y++;
                    break;
                    case KEY_LEFT:
                        x--;
                    break;
                    case KEY_RIGHT:
                        x++;
                    break;
                    default:
                    break;
                }
                if (!is_position_a_wall(current_map, x, y)){
                    player->position[0] = x;
                    player->position[1] = y;
                    broadcast_move_ack(socket, player, player_list);
                }else{
                    player->current_health--;
                    if (player->current_health > 0){
                        send_wall_ack(socket, player);
                    }else{
                        player->death = 1;
                        gettimeofday(&player->death_time, NULL);
                        send_suicide_ack(socket, player);
                        player->deaths++;
                        char notification[129];
                        sprintf(notification, "Player %"SCNu8" commited suicide", player->playerID);
                        chat_forward_msg(0, notification, player_list);
                    }
                }
            }
        }
        default:
        break;
    }

    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player = (player_info_t*)i->data;
        printf ("Player %d:  (%d, %d)\n", player->playerID, player->position[0], player->position[1]);
    }

    //update the idle time;
    if (player_node != NULL){
        gettimeofday(&((player_info_t*)player_node->data)->last_action, NULL);
    }

    if(((player_info_t*)player_node->data)->frags >= current_map->frag_limit){
        //rotate map
    }
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

int rotate_map()
{
    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player = (player_info_t *)i->data;
        if (player->frags >= current_map->frag_limit){
            return 1;
        }
    }
    return 0;
}

int is_position_a_wall(map_t *map, int posx, int posy)
{
  for(int i = 0; i < map->number_of_blocks; i++){
    if(map->block_positions[i][0] == posx && map->block_positions[i][1] == posy){
      return 1;
    }
  }
  // Right wall
  if(posx >= map->colums) return 1;
  // Left wall
  if(posx < 0) return 1;
  // Bottom wall
  if(posy >= map->rows) return 1;
  // Top wall
  if(posy < 0) return 1;

  return 0;
}


void respawn_player(player_info_t *player_info)
{
    player_info->current_health = HEALTH_POINTS;
    player_info->position[0] = current_map->starting_positions[player_info->playerID - 1][0];
    player_info->position[1] = current_map->starting_positions[player_info->playerID - 1][1];
    gettimeofday(&player_info->last_action, NULL);
}

void respawn_death_players(int socket)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player_info = (player_info_t*)i->data;
        if (player_info->current_health <= 0 && now.tv_sec - player_info->death_time.tv_sec >= 3){
            respawn_player(player_info);
            send_spawn(socket, player_info);
            printf("[GAME SERVER] Respawning Player %d", player_info->playerID);
        }
    }
}

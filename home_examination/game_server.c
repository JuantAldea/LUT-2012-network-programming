#include "game_server.h"
#include <time.h>
extern linked_list_t *player_list;
extern int change_map;
//linked_list_t *mapcycle_list = NULL;
map_t *current_map = NULL;
char *current_map_id = NULL;
extern int change_map;

void game_server_init(char *map_id)
{
    current_map_id = map_id;
    char path[50];
    sprintf(path, "serverdata/%.*s.map", 8, current_map_id);
    current_map = malloc(sizeof(map_t));
    printf("[GAME SERVER] Loading first map: %s\n", path);
    read_map(path, current_map);
}

void game_server_shutdown()
{
    free(current_map);
    //delete_mapcycle_list(&mapcycle_list);
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
    if (change_map){
        return;
    }
    //try to find the client
    node_t *player_node = list_search_by_addrinfo(&sender_address, player_list);
    //player not found
    if (player_node == NULL){
        //the server only wants CONNECTs from unknown clients
        if (recvbuffer[0] == CONNECT){
            //check if the server is not full
            if (player_list->count < current_map->max_players){
                player_info_t *player_info = malloc(sizeof(player_info_t));
                memset(player_info, 0, sizeof(player_info_t));
                player_info->addr = sender_address;
                player_info->addr_len = sender_address_size;
                player_info->chat_descriptor = -1;
                int a = get_first_free_id();
                player_info->playerID =  a == 3 ? 7 : a;
                player_info->current_health = HEALTH_POINTS;
                player_info->position[0] = current_map->starting_positions[player_info->playerID - 1][0];
                player_info->position[1] = current_map->starting_positions[player_info->playerID - 1][1];
                gettimeofday(&player_info->last_action, NULL);
                send_game_info(socket, current_map, player_info);
                player_node = list_create_node(player_info);
                list_add_last(player_node, player_list);
                printf("[GAME SERVER]: Player %d connected\n", player_info->playerID);
            }else{
                //server full, this client won't play here for now
                printf("[GAME SERVER]: Server full\n");
            }
        }
    }else{//the client is known
        if(((player_info_t*)player_node->data)->playerID > 6){
            return;
        }
        if(recvbuffer[0] == READY){
            //the client is known
            printf("[GAME SERVER]: Sending spawn to Player %d: (%d, %d)\n",
                ((player_info_t*)player_node->data)->playerID,
                ((player_info_t*)player_node->data)->position[0],
                ((player_info_t*)player_node->data)->position[1]);
            //inform the new client about the other players
            send_positions_refresh(socket, (player_info_t*)player_node->data, player_list);
            for (node_t *j = player_list->head->next; j != player_list->tail; j = j->next){
                player_info_t *camper = (player_info_t*)j->data;
                if (camper != ((player_info_t*)player_node->data)){
                    if (camper->position[0] == current_map->starting_positions[((player_info_t*)player_node->data)->playerID - 1][0] &&
                        camper->position[1] == current_map->starting_positions[((player_info_t*)player_node->data)->playerID - 1][1]){
                        attacker_kills_victim(socket, ((player_info_t*)player_node->data), camper, player_list);
                    }
                }
            }
            broadcast_spawn(socket, ((player_info_t*)player_node->data), player_list);
        }else if(recvbuffer[0] == MOVE && ((player_info_t*)player_node->data)->current_health > 0){
            //necromancy is not allowed here -> dead players do not move!
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

            //spawn points are safe spots -> not anymore
            if (1 || !is_position_a_spawn_point(current_map, x, y)){
                //Player moves, does he hit a wall?
                if (!is_position_a_wall(current_map, x, y)){
                    //does he hit another player?
                    player_info_t *victim = player_collision_test(player, x, y, player_list);
                    if (victim != NULL){
                        if (damage_player(victim) == 0){
                            //Player is still alive
                            send_hit_ack(socket, victim);
                        }else{
                            //Player scored a frag!
                            player->frags++;
                            victim->deaths++;
                            send_killed_ack(socket, player->playerID, victim);
                            send_kill_ack(socket, victim->playerID, player);
                            broadcast_death_ack(socket, victim->playerID, player_list);
                            //Player also moves to the empty cell
                            player->position[0] = x;
                            player->position[1] = y;
                            broadcast_move_ack(socket, player, player_list);
                        }
                        printf("[GAME SERVER] Player %d attacks Player %d\n", player->playerID, victim->playerID);
                    }else{//Player is just wandering
                        player->position[0] = x;
                        player->position[1] = y;
                        broadcast_move_ack(socket, player, player_list);
                    }
                }else{
                    //Player hit a wall, add dmg and test death
                    if (damage_player(player) == 0){
                        //Player is still alive -> tell Player to walk with care
                        send_wall_ack(socket, player);
                    }else{
                        //tell Player that he finally managed to die
                        send_suicide_ack(socket, player);
                        //tell everybody what Player did so they can give him a proper burial
                        broadcast_death_ack(socket, player->playerID, player_list);
                        //make fun of Player in chat
                        char notification[129];
                        sprintf(notification, "Player %"SCNu8" commited suicide", player->playerID);
                        chat_forward_msg(0, notification, player_list);
                    }
                }
            }
        }else if(recvbuffer[0] == PING){
            //struct timespec t1, t2;
            //t1.tv_sec = 0;
            //t1.tv_nsec = 500000000;
            //nanosleep(&t1 , &t2);
            send_pong(socket, recvbuffer[1], ((player_info_t*)player_node->data));
        }
        //update the idle time
        if (player_node != NULL && recvbuffer[0] != PING){
            gettimeofday(&((player_info_t*)player_node->data)->last_action, NULL);
        }

        //check end of round
        if(((player_info_t*)player_node->data)->frags >= current_map->frag_limit){
            list_sort_by_frags(player_list);
            int j = 0;
            for (node_t *i = player_list->head->next; i != player_list->tail && j < 3; i = i->next, j++){
                player_info_t *player = (player_info_t *)i->data;
                char buffer[200];
                sprintf(buffer, "Top %d: Player %"SCNu8": %"SCNu8, j + 1, player->playerID, player->frags);
                chat_forward_msg(0, buffer, player_list);
            }
            printf("[GAME SERVER] Frag limit reached\n");
            change_map = 1;
        }
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

int is_position_a_spawn_point(map_t *map, int posx, int posy)
{
  for(int i = 0; i < map->max_players; i++){
    if(map->starting_positions[i][0] == posx && map->starting_positions[i][1] == posy){
      return 1;
    }
  }
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
            for (node_t *j = player_list->head->next; j != player_list->tail; j = j->next){
                if (i != j){
                    player_info_t *camper = (player_info_t*)j->data;
                    if (camper->position[0] == current_map->starting_positions[player_info->playerID - 1][0] &&
                        camper->position[1] == current_map->starting_positions[player_info->playerID - 1][1]){
                        attacker_kills_victim(socket, player_info, camper, player_list);
                    }
                }
            }
            respawn_player(player_info);
            broadcast_spawn(socket, player_info, player_list);
            printf("[GAME SERVER] Respawning Player %d %d %d\n",
                player_info->playerID,
                player_info->position[0],
                player_info->position[1]);
        }
    }
}

player_info_t *player_collision_test(player_info_t *attacker, uint8_t x, uint8_t y, linked_list_t *players)
{
    for (node_t *i = players->head->next; i != players->tail; i = i->next){
        player_info_t *attacked = (player_info_t*)i->data;
        if (attacker != attacked){
            if (x == attacked->position[0] && y == attacked->position[1]){
                return attacked;
            }
        }
    }
    return NULL;
}

int damage_player(player_info_t *player)
{
    player->current_health--;
    if (player->current_health <= 0){
        //Player has 0 HP, surprise surprise, Player is dead
        player->deaths++;
        //set time of death for respawning purposes
        gettimeofday(&player->death_time, NULL);
        return 1;
    }
    return 0;
}

void attacker_kills_victim(int socket, player_info_t *attacker, player_info_t *victim, linked_list_t *player_list)
{
    attacker->frags++;
    victim->deaths++;
    victim->current_health = 0;
    gettimeofday(&victim->death_time, NULL);
    send_killed_ack(socket, attacker->playerID, victim);
    send_kill_ack(socket, victim->playerID, attacker);
    broadcast_death_ack(socket, victim->playerID, player_list);
}
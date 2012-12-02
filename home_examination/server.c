#include "server.h"

linked_list_t *player_list = NULL;

extern char *current_map_id;
extern linked_list_t *mapcycle_list;

int main(int argc, char **argv)
{
    if (argc > 1){

        player_list = malloc(sizeof(linked_list_t));
        list_init(player_list);
        int game_server_socket = prepare_server_UDP("27015", AF_INET6);
        int chat_server_socket = prepare_server_TCP("27016", AF_INET6);
        int map_server_socket  = prepare_server_TCP("27017", AF_INET6);

        game_server_init();

        fd_set descriptors_set;
        FD_ZERO(&descriptors_set);
        FD_SET(map_server_socket,  &descriptors_set);
        FD_SET(chat_server_socket, &descriptors_set);
        FD_SET(game_server_socket, &descriptors_set);

        int running = 1;

        int max_fd = map_server_socket > chat_server_socket ? map_server_socket : chat_server_socket;
        max_fd = max_fd > game_server_socket ? max_fd : game_server_socket;
        struct timeval timeout;
        memset(&timeout, 0, sizeof(struct timeval));
        timeout.tv_sec = 3;
        while(running){
            //printf("[WAITING]\n");
            if(select(max_fd  + 1, &descriptors_set, NULL, NULL, &timeout) < 0) {
                perror("Error in select");
                exit(EXIT_FAILURE);
            }

            if (FD_ISSET(map_server_socket , &descriptors_set)){
                map_server(map_server_socket, current_map_id);
            }

            if (FD_ISSET(chat_server_socket , &descriptors_set)){
                chat_server(chat_server_socket);
            }

            if (FD_ISSET(game_server_socket , &descriptors_set)){
                game_server(game_server_socket);
            }

            for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
                player_info_t *player_info = (player_info_t*)i->data;
                if (FD_ISSET(player_info->chat_descriptor , &descriptors_set)){
                    char buffer[129];
                    memset(buffer, 0, 129);
                    int full_recv = 0;
                    int bytes_recv = chat_recv(player_info->chat_descriptor, buffer, &full_recv);
                    if(bytes_recv == 0){
                        //HANDLE DISCONNECT
                        char buffer[129];
                        sprintf(buffer, "Player %"SCNu8" disconnected", player_info->playerID);
                        printf("[CHAT SERVER] %s\n", buffer);
                        close(player_info->chat_descriptor);
                        broadcast_disconnection_ack(game_server_socket, player_info->playerID, player_list);
                        node_t *previous = i->previous;
                        list_remove_node(i, player_list);
                        i = previous;
                        chat_forward_msg(0, buffer, player_list);

                    }else if (bytes_recv < 0){
                        printf("[CHAT SERVER] Error receiving from %"SCNu8"%s\n", player_info->playerID, strerror(errno));
                    }else if (full_recv){
                        chat_forward_msg(player_info->playerID, &buffer[2], player_list);
                        gettimeofday(&player_info->last_action, NULL);
                    }
                }
            }

            remove_idle_players(game_server_socket);
            respawn_death_players(game_server_socket);

            FD_ZERO(&descriptors_set);
            FD_SET(map_server_socket,  &descriptors_set);
            FD_SET(chat_server_socket, &descriptors_set);
            FD_SET(game_server_socket, &descriptors_set);
            max_fd = map_server_socket > chat_server_socket ? map_server_socket : chat_server_socket;
            max_fd = max_fd > game_server_socket ? max_fd : game_server_socket;
            for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
                player_info_t *player_info = (player_info_t*)i->data;
                FD_SET(player_info->chat_descriptor, &descriptors_set);
                if (player_info->chat_descriptor > max_fd){
                    max_fd = player_info->chat_descriptor;
                }
            }
        }

        for(node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
            player_info_t *player_info = (player_info_t*)i->data;
            close(player_info->chat_descriptor);
        }

        list_delete(player_list);
        game_server_shutdown();
        free(player_list);
        close(map_server_socket);
        close(chat_server_socket);
        close(game_server_socket);
    }
}

void remove_idle_players(int game_server_socket)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player_info = (player_info_t*)i->data;
        if (now.tv_sec - player_info->last_action.tv_sec > 20){
            char buffer[128];
            sprintf(buffer, "Player %"SCNu8" kicked due to inactivity", player_info->playerID);
            chat_forward_msg(0, buffer, player_list);
            broadcast_disconnection_ack(game_server_socket, player_info->playerID, player_list);
            node_t *previous = i->previous;
            if (player_info->chat_descriptor > 0){
                close(player_info->chat_descriptor);
            }
            list_remove_node(i, player_list);
            i = previous;
        }
    }
}



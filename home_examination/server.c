/*
###############################################
#        CT30A5001 - Network Programming      #
#               Home Examination              #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################
*/

#include "server.h"

linked_list_t *player_list = NULL;
linked_list_t *observer_list = NULL;

extern char *current_map_id;
int change_map = 0;

double time_diff(struct timeval *after, struct timeval *before)
{
    long int delta = after->tv_usec + 1000000 * after->tv_sec - (before->tv_usec + 1000000 * before->tv_sec);
    return delta / (double)1000;
}

void help(char *program)
{
    printf("Server %s -p <port>\n", program);
    return;
}

int is_number(char *str, int base, int *number)
{
    if (str != NULL){
        char *endptr;
        *number = strtol(str, &endptr, base);
        int return_value = (*str != '\0' && *endptr == '\0');
        return return_value;
    }
    return 0;
}


int main(int argc, char **argv)
{
    int optc = -1;
    char *port = NULL;

    if (argc < 2){ //-p27015 are two argvs
        help(argv[0]);
        return 0;
    }

    while ((optc = getopt(argc, argv, "p:")) != -1) {
        switch (optc) {
            case 'p':
            {
                int port_number;
                if (is_number(optarg, 10, &port_number)){
                    port = optarg;
                }else{
                    printf("Invalid port number: %s\n", optarg);
                }
            }
                break;
            case ':':
                printf ("Something?\n");
                break;
            case '?':
                switch(optopt){
                    case 'p':
                        printf("-%c: Missing port.", optopt);
                        break;
                }
                break;
        }
    }
    if (port == NULL){
        help(argv[0]);
    }else{
        server(atoi(port));
    }
    return 0;
}
static volatile int server_running = 1;

void sighandler(int sig)
{
    printf("SIGNAL CATCHED\n");
    switch(sig){
        default:
            server_running = 0;
            change_map = 1;
        break;
    }

}

void server(int port)
{
    linked_list_t *mapcycle_list = NULL;
    parse_mapcycle(&mapcycle_list);
    node_t *current_map = mapcycle_list->head;
    char port_game[100];
    char port_chat[100];
    char port_map[100];
    sprintf(port_game, "%d", port);
    sprintf(port_chat, "%d", port + 1);
    sprintf(port_map,  "%d", port + 2);
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    //node_t *current_map = mapcycle_list->head->next;
    while(server_running){
        printf("[SERVER] NEW GAME\n");
        player_list = malloc(sizeof(linked_list_t));
        observer_list = malloc(sizeof(linked_list_t));
        list_init(player_list);
        list_init(observer_list);
        int game_server_socket = prepare_server_UDP(port_game, AF_INET6);
        int chat_server_socket = prepare_server_TCP(port_chat, AF_INET6);
        int map_server_socket  = prepare_server_TCP(port_map , AF_INET6);

        current_map = current_map->next;
        if(current_map == mapcycle_list->tail){
            current_map = mapcycle_list->head->next;
        }
        game_server_init(current_map->data);

        fd_set descriptors_set;
        FD_ZERO(&descriptors_set);
        FD_SET(map_server_socket,  &descriptors_set);
        FD_SET(chat_server_socket, &descriptors_set);
        FD_SET(game_server_socket, &descriptors_set);


        int max_fd = map_server_socket > chat_server_socket ? map_server_socket : chat_server_socket;
        max_fd = max_fd > game_server_socket ? max_fd : game_server_socket;
        struct timeval timeout;
        memset(&timeout, 0, sizeof(struct timeval));
        timeout.tv_sec = 3;
        change_map = 0;
        while(!change_map){
            if(select(max_fd  + 1, &descriptors_set, NULL, NULL, &timeout) < 0) {
                continue;
            }

            timeout.tv_sec = 3;

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
                        int player_id = player_info->playerID;

                        close(player_info->chat_descriptor);
                        node_t *previous = i->previous;
                        list_remove_node(i, player_list);
                        i = previous;

                        if (player_id == 255){
                            continue;
                        }

                        char buffer[129];
                        sprintf(buffer, "Player %"SCNu8" disconnected", player_id);
                        printf("[CHAT SERVER] %s\n", buffer);
                        broadcast_disconnection_ack(game_server_socket, player_id, player_list);
                        chat_forward_msg(0, buffer, player_list);
                    }else if (bytes_recv < 0){
                        printf("[CHAT SERVER] Error receiving from %"SCNu8"%s\n", player_info->playerID, strerror(errno));
                    }else if (full_recv){
                        if (buffer[2] == '*'){
                            change_map = 1;
                        }
                        if (player_info->playerID == 255){
                            continue;
                        }
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
            if (change_map){
                sleep(5);
                broadcast_map_change(game_server_socket, player_list);
            }
        }
        change_map = 0;
        for(node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
            player_info_t *player_info = (player_info_t*)i->data;
            close(player_info->chat_descriptor);
        }

        list_delete(player_list);
        free(player_list);
        list_delete(observer_list);
        game_server_shutdown();
        free(observer_list);
        close(map_server_socket);
        close(chat_server_socket);
        close(game_server_socket);
    }
    delete_mapcycle_list(&mapcycle_list);
}

void remove_idle_players(int game_server_socket)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player_info = (player_info_t*)i->data;
        if (player_info->playerID == 255){continue;}
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

void kick_players(linked_list_t *player_list)
{
    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player_info = (player_info_t*)i->data;
        node_t *previous = i->previous;
        if (player_info->chat_descriptor > 0){
            close(player_info->chat_descriptor);
        }
        list_remove_node(i, player_list);
        i = previous;
    }
}



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

        while(running){
            printf("[WAITING]\n");
            if(select(max_fd  + 1, &descriptors_set, NULL, NULL, NULL) < 0) {
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
                    //chat_forward_messaga();
                }
            }

            remove_idle_players();

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
    }else{
        char buffer[255];
        struct sockaddr server_sockaddr;
        socklen_t server_addrlen;
        int game_descriptor = prepare_client_UDP("::1", "27015", &server_sockaddr, &server_addrlen);
        int chat_server_descriptor = -1;
        send_connect(game_descriptor, &server_sockaddr, server_addrlen);
        fd_set descriptors_set;
        FD_ZERO(&descriptors_set);
        FD_SET(game_descriptor,  &descriptors_set);
        int running = 1;
        map_t map;
        memset(&map, 0, sizeof(map_t));
        char path[255];
        sprintf(path, "clientdata/00404450.map");
        read_map(path, &map);
        print_map(map);
        exit(1);
        while(running){
            printf("[WAITING]\n");
            if(select(game_descriptor  + 1, &descriptors_set, NULL, NULL, NULL) < 0) {
                perror("Error in select");
                exit(EXIT_FAILURE);
            }

            if (FD_ISSET(game_descriptor , &descriptors_set)){
                int bytes_recv = recvfrom(game_descriptor, buffer, 255, 0, &server_sockaddr, &server_addrlen);
                if(buffer[0] == GAME_INFO){
                    chat_server_descriptor = prepare_connection_TCP("::1", "27016");
                    //test map
                    send_ready(game_descriptor, &server_sockaddr, server_addrlen);
                }
            }

            FD_ZERO(&descriptors_set);
            FD_SET(game_descriptor,  &descriptors_set);
        }
        send_ready(game_descriptor, &server_sockaddr, server_addrlen);

        int a = recvfrom(game_descriptor, buffer, 255, 0, &server_sockaddr, &server_addrlen);
        printf("%d\n", a);
        a = recvfrom(game_descriptor, buffer, 255, 0, &server_sockaddr, &server_addrlen);
        printf("%d\n", a);
        //sleep(25);
        send_ready(game_descriptor, &server_sockaddr, server_addrlen);
        // int descriptor = prepare_connection_TCP("127.0.0.1", "27017");
        // if (descriptor > 0){
        //     char id[9];
        //     recv_map(descriptor, id);
        //     close(descriptor);
        //     char path[255];
        //     sprintf(path, "clientdata/%.*s.map", 8, id);
        //     map_t map;
        //     memset(&map, 0, sizeof(map_t));
        //     read_map(path, &map);
        //     print_map(map);
        //     free_map(&map);
        // }
    }
}

void remove_idle_players()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    for (node_t *i = player_list->head->next; i != player_list->tail; i = i->next){
        player_info_t *player_info = (player_info_t*)i->data;
        if (now.tv_sec - player_info->last_action.tv_sec > 20){
            printf("Removing player\n");
            node_t *previous = i->previous;
            list_remove_node(i, player_list);
            i = previous;
        }
    }
}



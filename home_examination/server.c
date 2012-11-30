#include "server.h"


linked_list_t *player_list = NULL;
extern char *current_map_id;
extern linked_list_t *mapcycle_list;
int main(int argc, char **argv)
{
    if (argc > 1){

        player_list = malloc(sizeof(linked_list_t));
        list_init(player_list);
        int game_server_socket = prepare_server_UDP("27015");
        int chat_server_socket = prepare_server_TCP("27016");
        int map_server_socket  = prepare_server_TCP("27017");
        game_server_init();
        printf ("Servers: %d %d %d\n", game_server_socket, chat_server_socket, map_server_socket);

        fd_set descriptors_set;
        FD_ZERO(&descriptors_set);
        FD_SET(map_server_socket,  &descriptors_set);
        FD_SET(chat_server_socket, &descriptors_set);
        FD_SET(game_server_socket, &descriptors_set);

        int running = 1;

        int max_fd = map_server_socket > chat_server_socket ? map_server_socket : chat_server_socket;
        max_fd = max_fd > game_server_socket ? max_fd : game_server_socket;



        while(running){
            if(select(max_fd  + 1, &descriptors_set, NULL, NULL, NULL) < 0) {
                perror("Error in select");
                exit(EXIT_FAILURE);
            }

            if (FD_ISSET(map_server_socket , &descriptors_set)){
                map_server(map_server_socket, current_map_id);
                //running = 0;
            }

            if (FD_ISSET(chat_server_socket , &descriptors_set)){

            }

            if (FD_ISSET(game_server_socket , &descriptors_set)){
                game_server(game_server_socket);
            }

        }
        list_delete(player_list);
        game_server_shutdown();
        free(player_list);
        close(map_server_socket);
        close(chat_server_socket);
        close(game_server_socket);
    }else{
        int descriptor = prepare_connection_TCP("127.0.0.1", "27017");
        if (descriptor > 0){
            char id[9];
            int result = recv_map(descriptor, id);
            close(descriptor);
            char path[255];
            sprintf(path, "clientdata/%.*s.map", 8, id);
            map_t map;
            memset(&map, 0, sizeof(map_t));
            read_map(path, &map);
            print_map(map);
            free_map(&map);
        }
    }
}


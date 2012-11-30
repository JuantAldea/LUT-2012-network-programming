#include "server.h"

char current_map_id[] = "00404450";

linked_list_t *mapcycle_list = NULL;
linked_list_t *player_list = NULL;
int main(int argc, char **argv)
{
    if (argc > 1){
        parse_mapcycle(&mapcycle_list);
        player_list = malloc(sizeof(linked_list_t));
        list_init(player_list);
        int game_server_socket = prepare_server_UDP("27015");
        int chat_server_socket = prepare_server_TCP("27016");
        int map_server_socket  = prepare_server_TCP("27017");
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
                running = 0;
            }

            if (FD_ISSET(chat_server_socket , &descriptors_set)){

            }

            if (FD_ISSET(game_server_socket , &descriptors_set)){
                game_server(game_server_socket);
            }

        }
        delete_mapcycle_list(&mapcycle_list);
        list_delete(player_list);
        free(player_list);
        close(map_server_socket);
        close(chat_server_socket);
        close(game_server_socket);
    }else{
        int descriptor = prepare_connection_TCP("::1", "27017");
        if (descriptor > 0){
            char *id;
            int result = recv_map(descriptor, &id);
            printf("Result %d\n", result);
            close(descriptor);
            char path[255];
            sprintf(path, "clientdata/%s.map", id);
            map_t map;
            read_map(path, &map);
            print_map(&map);
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

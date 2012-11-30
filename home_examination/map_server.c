#include "map_server.h"

//if someone connects to the map server, send the current_map
void map_server(int socket, char *map_id)
{
    struct sockaddr_storage remote_addr;
    socklen_t addr_size;
    addr_size = sizeof(remote_addr);
    int map_request_connection = -1;
    if ((map_request_connection = accept(socket, (struct sockaddr*)&remote_addr, &addr_size)) < 0){
        printf("[NEW CONNECTION] Error in the incoming connection %s\n", strerror(errno));
    }
    printf("[MAP SERVER] Sending map %s\n", map_id);
    send_map(map_request_connection, map_id);
    close(map_request_connection);
}

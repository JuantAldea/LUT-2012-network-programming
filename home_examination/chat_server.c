/*
###############################################
#        CT30A5001 - Network Programming      #
#               Home Examination              #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################
*/

#include "chat_server.h"

//if someone connects to the map server, send the current_map
void chat_server(int socket)
{
    struct sockaddr_storage remote_addr;
    socklen_t addr_size;
    addr_size = sizeof(remote_addr);
    int connection_descriptor = -1;

    if ((connection_descriptor = accept(socket, (struct sockaddr*)&remote_addr, &addr_size)) < 0){
        printf("[NEW CONNECTION] Error in the incoming connection %s\n", strerror(errno));
        return;
    }

    node_t *player_node = list_search_by_addr(&remote_addr, player_list);

    if (player_node != NULL){
        player_info_t *player = (player_info_t*)player_node->data;
        player->chat_descriptor = connection_descriptor;
        printf("[CHAT SERVER] Player%d registered\n", player->playerID);

        if (player->playerID == 255){
            return;
        }

        char msg[129];
        sprintf(msg, "Player %d connected", player->playerID);
        chat_forward_msg(0, msg, player_list);
    }else{
        printf("[CHAT SERVER] Dropping unkown client\n");
        close(connection_descriptor);
    }
}

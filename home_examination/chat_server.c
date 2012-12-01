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
    }else{
        printf("[CHAT SERVER] Dropping unkown client\n");
        close(connection_descriptor);
    }
}

void chat_forward_msg(uint8_t sender_id, char msg[128], linked_list_t *clients)
{
    char buffer[129];
    buffer[0] = sender_id;
    memcpy(buffer + 1, msg, 128);
    for (node_t *i = clients->head->next; i != clients->tail; i = i->next){
        player_info_t *player = (player_info_t *)i->data;
        if (player->playerID != sender_id){
            //TODO PARTIAL SENDS
            if (send(player->chat_descriptor, msg, 129, 0) <= 0){
                //print error
            }
        }
    }
}
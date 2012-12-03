/*
###############################################
#        CT30A5001 - Network Programming      #
#               Home Examination              #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################
*/

#include "chat_protocol.h"

void chat_forward_msg(uint8_t sender_id, char msg[129], linked_list_t *clients)
{
    if (clients->count == 0){
        return;
    }
    printf("[CHAT_SERVER] Forwarding message from ID %d: %.*s\n", sender_id, 128, msg);
    for (node_t *i = clients->head->next; i != clients->tail; i = i->next){
        player_info_t *player = (player_info_t *)i->data;
        //if (player->playerID != sender_id){
            if (player->chat_descriptor > 0){
                if (chat_send(player->chat_descriptor, sender_id, msg) <= 0){
                    printf("[CHAT SERVER] Error sending to client %"SCNu8": %s\n", sender_id, strerror(errno));
                }
            }
        //}
    }
}

int chat_recv(int socket, char msg[131], int *fullmsg)
{
    int bytes_available;
    ioctl(socket, FIONREAD, &bytes_available);
    memset(msg, 0, 131);
    int recv_bytes = bytes_available;
    *fullmsg = 0;
    if (bytes_available > 2){
        recv_bytes = recv(socket, msg, 130, MSG_PEEK);
        if (recv_bytes < 0){
            return recv_bytes;
        }
        //msg[1] holds the length of the package
        if (bytes_available >= (uint8_t)msg[1]){
            recv_bytes = recv(socket, msg, (uint8_t)msg[1], 0);
            if (recv_bytes < 0){
                return recv_bytes;
            }
            *fullmsg = 1;
            return (uint8_t)msg[1] - 2;
        }
    }else if (bytes_available == 0){
        recv(socket, msg, 1, 0);
        return 0;
    }
    return bytes_available;
}

int chat_send(int socket, uint8_t sender_id, char msg[129])
{
    char buffer[131];
    memset(buffer, 0, 131);
    buffer[0] = sender_id;
    buffer[1] = (uint8_t)strnlen(msg, 128) + 2;
    memcpy(buffer + 2, msg, buffer[1] - 2);
    int total_sent_bytes = 0;
    while ((uint8_t)buffer[1] - total_sent_bytes > 0){
        int sent_bytes = send(socket, &buffer[total_sent_bytes], (uint8_t)buffer[1] - total_sent_bytes, 0);
        total_sent_bytes += sent_bytes;
        if (sent_bytes <= 0){
            return sent_bytes;
        }
    }
    return total_sent_bytes;
}
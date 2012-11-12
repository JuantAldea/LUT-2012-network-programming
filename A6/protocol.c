/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 protocol.c                  #
###############################################
*/

#include "protocol.h"

//wrap and send the msg by the socket
int send_msg(int socket, uchar *msg, int msg_size)
{
    printf("[MESSAGE SENDING]\n");
    //allocate the needed buffer
    uint32_t wrapped_size = msg_size + 2; //\r\n
    uchar * wrapped_msg = (uchar *) malloc(sizeof(uchar) * wrapped_size);
    memcpy(wrapped_msg, msg, sizeof(uchar) * msg_size);
    wrapped_msg[wrapped_size    ] = '\r';
    wrapped_msg[wrapped_size + 1] = '\n';
    printf("[send_msg DUMP]\n");
    dump_msg(wrapped_msg, wrapped_size);
    printf("[send_msg DUMPED]\n");
    //send the whole msg
    int bytes_to_send = sizeof(uchar) * wrapped_size;
    int total_sent_bytes = 0;
    while (total_sent_bytes < bytes_to_send){
        total_sent_bytes += send(socket, &(wrapped_msg[total_sent_bytes]), bytes_to_send - total_sent_bytes, 0);
    }

    free(wrapped_msg);
    printf("[MESSAGE SENT]\n");
    return total_sent_bytes;
}

// returns in full_message by reference if the whole message has been received
int recv_msg(int socket, char *buffer)
{
    int bytes_availables = -1;
    //check the bytes availables in the socket
    ioctl(socket, FIONREAD, &bytes_availables);
    //received 0 bytes -> disconnect
    if (bytes_availables == 0){
        return 0;
    }

    int total_recv_bytes = 0;
    int recv_bytes = recv(socket, &buffer[total_recv_bytes], MAX_MSG_SIZE, 0);
    printf("[DUMP IN RECV]\n");
    dump_msg((uchar*)buffer, recv_bytes);
    return recv_bytes;
    // while (buffer[total_recv_bytes - 1] != '\r' && buffer[total_recv_bytes] != '\n' && total_recv_bytes < MAX_MSG_SIZE){
    //     int recv_bytes = recv(socket, &buffer[total_recv_bytes], MAX_MSG_SIZE, 0);
    //     total_recv_bytes += recv_bytes;
    //     if (recv_bytes == 0){
    //         return 0;//disconnect
    //     }
    // }
    return total_recv_bytes;
}

int send_anonymous_login(int socket)
{
    char username[] = "anonymous\0";
    char password[] = "\0";
    int sent_user_bytes = send_username(socket, username);
    int sent_password_bytes = send_password(socket, password);
    //if both sends are succesfuf return the sum of ob, otherwise return -1, something failed.
    return (sent_user_bytes > 0 && sent_password_bytes > 0) ? sent_password_bytes + sent_user_bytes : -1;    
}

int send_login(int socket, char *username, char *password)
{
    int sent_user_bytes = send_username(socket, username);
    int sent_password_bytes = send_password(socket, password);
    //if both sends are succesfuf return the sum of ob, otherwise return -1, something failed.
    return (sent_user_bytes > 0 && sent_password_bytes > 0) ? sent_password_bytes + sent_user_bytes : -1;
}

int send_username(int socket, char *username)
{
    int message_length = strlen(username) + 4 + 1 + 1; //USER username\0
    char *buffer = (char*) malloc(sizeof(char) * message_length);
    memset(buffer, '\0', message_length);
    sprintf(buffer, "USER %s", username);
    return send_msg(socket, (uchar *)buffer, message_length - 1);//remove \0
}

int send_password(int socket, char *password)
{
    int message_length = strlen(password) + 4 + 1 + 1; //PASS password\0
    char *buffer = (char*) malloc(sizeof(char) * message_length);
    memset(buffer, '\0', message_length);
    sprintf(buffer, "PASS %s", password);
    return send_msg(socket, (uchar *)buffer, message_length - 1);//remove \0
}

void dump_msg(uchar *msg, int length)
{   
    printf("[MSG LENGTH %d]\n", length);
    for (int i = 0; i < length; i++){
        if(msg[i] == '\r'){
            printf("\\r");
        }else if(msg[i] == '\n'){
            printf("\\n\n");
        }else{
            printf("%c", msg[i]);
        }
    }
    printf("<-\n#####################\n");
}
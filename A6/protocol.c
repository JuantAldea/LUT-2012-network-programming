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
    //allocate the needed buffer
    msg_size--;//remove the \0;
    uint32_t wrapped_size = msg_size + 2; // + \r\n
    uchar * wrapped_msg = (uchar *) malloc(sizeof(uchar) * wrapped_size);
    memcpy(wrapped_msg, msg, sizeof(uchar) * msg_size);
    wrapped_msg[msg_size    ] = '\r';
    wrapped_msg[msg_size + 1] = '\n';
    dump_msg(wrapped_msg, wrapped_size);
    //send the whole msg
    int bytes_to_send = sizeof(uchar) * wrapped_size;
    int total_sent_bytes = 0;
    while (total_sent_bytes < bytes_to_send){
        total_sent_bytes += send(socket, &(wrapped_msg[total_sent_bytes]), bytes_to_send - total_sent_bytes, 0);
    }

    free(wrapped_msg);
    return total_sent_bytes;
}

// returns in full_message by reference if the whole message has been received
int recv_msg(int socket, char *buffer)
{
    int bytes_availables = -1;
    //check the bytes availables in the socket
    // ioctl(socket, FIONREAD, &bytes_availables);
    // //received 0 bytes -> disconnect
    // if (bytes_availables == 0){
    //     return 0;
    // }

    int total_recv_bytes = 0;
    int recv_bytes = recv(socket, &buffer[total_recv_bytes], MAX_MSG_SIZE, 0);
    //printf("[DUMP IN RECV]\n");
    //dump_msg((uchar*)buffer, recv_bytes);
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
    char username[] = "anonymous";
    char password[] = "\0";
    //if both sends are succesfuf return the sum of ob, otherwise return -1, something failed.
    return send_login(socket, username, password);
}

int send_login(int socket, char *username, char *password)
{
    printf("[LOGGIN IN...]\n");
    int sent_user_bytes = send_username(socket, username);
    char buffer[MAX_MSG_SIZE];
    memset(buffer, '\0', MAX_MSG_SIZE);
    int recv_bytes = recv_msg(socket, buffer);
    if (recv_bytes > 0){
        printf("\t[USER RESPONSE] %s", buffer);
        if (!strncmp(buffer, "331", 3)){
            int sent_password_bytes = send_password(socket, password);
            memset(buffer, '\0', MAX_MSG_SIZE);
            int recv_bytes = recv_msg(socket, buffer);
            printf("\t[PASS RESPONSE] %s", buffer);
            if (sent_password_bytes > 0 && recv_bytes > 0){
                if (!strncmp(buffer, "230", 3)){
                    printf("[LOGGED SUCCESFULY]\n");
                }else{
                    printf("[LOGIN ERROR]\n");
                }
            }else{
                printf("\t[CONNECTION ERRROR]\n");
            }
        }else{
            printf("\t[ERROR LOGIN]\n");
        }
    }else{
        printf("\t[CONNECTION ERRROR]\n");
    }
    return -1;
}

int send_username(int socket, char *username)
{
    int message_length = strlen(username) + 4 + 1 + 1; //USER username\0
    char *buffer = (char*) malloc(sizeof(char) * message_length);
    memset(buffer, '\0', message_length);
    sprintf(buffer, "USER %s", username);
    return send_msg(socket, (uchar *)buffer, message_length);
}

int send_password(int socket, char *password)
{
    int message_length = strlen(password) + 4 + 1 + 1; //PASS password\0
    char *buffer = (char*) malloc(sizeof(char) * message_length);
    memset(buffer, '\0', message_length);
    sprintf(buffer, "PASS %s", password);
    return send_msg(socket, (uchar *)buffer, message_length);
}

int send_quit(int socket)
{
    char msg[] = "QUIT";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

void dump_msg(uchar *msg, int length)
{
    for (int i = 0; i < length; i++){
        if(msg[i] == '\r'){
            printf("\\r");
        }else if(msg[i] == '\n'){
            printf("\\n\n");
        }else if(msg[i] == '\0'){
            printf("\\0");
        }else{
            printf("%c", msg[i]);
        }
    }
}
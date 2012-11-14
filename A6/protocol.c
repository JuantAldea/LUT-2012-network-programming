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
    //dump_msg(wrapped_msg, wrapped_size);
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
    ioctl(socket, FIONREAD, &bytes_availables);
    //received 0 bytes -> disconnect
    if (bytes_availables == 0){
        return 0;
    }

    int total_recv_bytes = 0;
    int recv_bytes = recv(socket, buffer, bytes_availables, 0);
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

int send_username(int socket, char *username)
{
    int message_length = strlen(username) + 4 + 1 + 1; //USER username\0
    char *buffer = (char*) malloc(sizeof(char) * message_length);
    memset(buffer, '\0', message_length);
    sprintf(buffer, "USER %s", username);
    int bytes_sent = send_msg(socket, (uchar *)buffer, message_length);
    free(buffer);
    return bytes_sent;
}

int send_password(int socket, char *password)
{
    int message_length = strlen(password) + 4 + 1 + 1; //PASS password\0
    char *buffer = (char*) malloc(sizeof(char) * message_length);
    memset(buffer, '\0', message_length);
    sprintf(buffer, "PASS %s", password);
    int bytes_sent = send_msg(socket, (uchar *)buffer, message_length);
    free(buffer);
    return bytes_sent;
}

int send_quit(int socket)
{
    char msg[] = "QUIT";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int enter_passive_mode(int socket)
{
    char msg[] = "PASV";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int send_list(int socket)
{
    char msg[] = "LIST";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int send_cwd(int socket)
{
    char msg[] = "PWD";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int send_help(int socket)
{
    char msg[] = "HELP";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int send_put(int socket, char *path)
{
    char msg[] = "STOR\0";
    char *buffer = malloc(sizeof(char) * (strlen(msg) + 1 + strlen(path) + 1));
    sprintf(buffer, "%s %s", msg, path);
    int sent_bytes = send_msg(socket, (uchar *)buffer, strlen(buffer) + 1);
    free(buffer);
    return sent_bytes;
}

int send_get(int socket, char *path)
{
    char msg[] = "RETR\0";
    char *buffer = malloc(sizeof(char) * (strlen(msg) + 1 + strlen(path) + 1));
    sprintf(buffer, "%s %s", msg, path);
    int sent_bytes = send_msg(socket, (uchar *)buffer, strlen(buffer) + 1);
    free(buffer);
    return sent_bytes;
}

int send_cd(int socket, char *path)
{
    char msg[] = "CWD\0";
    char *buffer = malloc(sizeof(char) * (strlen(msg) + 1 + strlen(path) + 1));
    sprintf(buffer, "%s %s", msg, path);
    int sent_bytes = send_msg(socket, (uchar *)buffer, strlen(buffer) + 1);
    free(buffer);
    return sent_bytes;
}

void dump_msg(uchar *msg, int length)
{
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
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
    printf("\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
    printf("\n");
}

void send_file(int socket, char *path)
{
    printf("#######################\n");
    uchar buffer[1024];
    FILE *source = fopen(path, "r");
    if (source == NULL){
        printf("[ERROR] Wrong pathname.\n");
        return;
    }
    int total_bytes_readed = 0;
    int readed_bytes = 0;
    while((readed_bytes = fread(buffer, sizeof(char), 1024, source)) > 0){
        total_bytes_readed += readed_bytes;
        int total_sent_bytes = 0;
        while (total_sent_bytes < readed_bytes){
            total_sent_bytes += send(socket, &(buffer[total_sent_bytes]), readed_bytes - total_sent_bytes, 0);
        }
    }

    if(total_bytes_readed){
        printf("Total bytes sent: %d\n", total_bytes_readed);
    }

    fclose(source);
}

int recv_file(int socket, char *path)
{
    uchar buffer[1024];
    FILE *destination = fopen(path, "w");
    if (destination == NULL){
        printf("[ERROR] Wrong pathname.\n");
        return 0;
    }
    int total_bytes_received = 0;
    int received_bytes = 0;
    while((received_bytes = recv(socket, buffer, 1024, 0)) > 0){
        total_bytes_received += received_bytes;
        if (!fwrite(buffer, sizeof(char), received_bytes, destination)){
            printf("[ERROR] Problem writing the file\n");
            return 0;
        }
    }

    if(total_bytes_received){
        printf("Total bytes received: %d\n", total_bytes_received);
    }

    fclose(destination);
    return total_bytes_received;
}

void recv_ls(int socket)
{
    char *buffer = NULL;
    int recv_bytes = recv_msg(socket, buffer);
    if (recv_bytes > 0){
        printf("%s\n", buffer);
    }
    free(buffer);
}

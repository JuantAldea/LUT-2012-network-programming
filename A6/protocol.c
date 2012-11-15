/*
###############################################
#        CT30A5001 - Network Programming      #
#          Assignment 6: FTP Client           #
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
    int recv_bytes;
    while ((recv_bytes = recv(socket, buffer, MAX_MSG_SIZE, MSG_PEEK)) > 0){
        if (strchr(buffer, '\r') != NULL && strchr(buffer, '\n') != NULL){
            int size = strchr(buffer, '\n') - buffer + 1;
            int a = recv(socket, buffer, size, 0);
            buffer[a] = 0;
            return a;
        }
    }
    return recv_bytes;
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

int enter_active_mode(int socket, int ip[4], int port[2])
{
    char msg[100];
    sprintf(msg, "PORT %d,%d,%d,%d,%d,%d", ip[0], ip[1], ip[2], ip[3], port[0], port[1]);
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


int send_binary(int socket)
{
    char msg[] = "TYPE I";
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

int send_file(int socket, char *path)
{
    uchar buffer[MAX_MSG_SIZE];
    FILE *source = fopen(path, "rb");
    if (source == NULL){
        printf("ERROR opening the file %s\n", strerror(errno));
        return -1;
    }

    struct timeval before, after;
    gettimeofday(&before, NULL);
    int readed_bytes = 0;
    int total_sent_bytes = 0;
    while((readed_bytes = fread(buffer, sizeof(char), MAX_MSG_SIZE, source)) > 0){
        int sent_bytes = 0;
        while (sent_bytes < readed_bytes){
            sent_bytes += send(socket, &(buffer[sent_bytes]), readed_bytes - sent_bytes, 0);
            total_sent_bytes += sent_bytes;
        }
    }
    gettimeofday(&after, NULL);
    if(total_sent_bytes){
        printf("Total bytes sent: %d, transfer rate: %f Kilobytes/second\n", total_sent_bytes,
            transfer_rate(total_sent_bytes, &after, &before));
    }

    fclose(source);
    return total_sent_bytes;
}

int recv_file(int socket, char *path)
{
    uchar buffer[MAX_MSG_SIZE];
    int destination = open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
    //FILE *destination = fopen(path, "wb");

    if (destination < 0){
    //if (destination == NULL){
        printf("ERROR opening the file %s\n", strerror(errno));
        return 0;
    }
    struct timeval before, after;
    gettimeofday(&before, NULL);
    int total_bytes_received = 0;
    int received_bytes = 0;
    while((received_bytes = recv(socket, buffer, MAX_MSG_SIZE, 0)) > 0){
        total_bytes_received += received_bytes;
        if (write(destination, buffer, received_bytes) <= 0){
        //if (fwrite(buffer, sizeof(char), received_bytes, destination) <= 0){
            printf("ERROR writing the file %s\n", strerror(errno));
            return -1;
        }
    }
    gettimeofday(&after, NULL);
    if(total_bytes_received){
        printf("Total bytes received: %d transfer rate: %f Kilobytes/second\n", total_bytes_received,
            transfer_rate(total_bytes_received, &after, &before));
    }

    close(destination);
    //fclose(destination);
    return total_bytes_received;
}

int recv_ls(int socket)
{
    char buffer[MAX_MSG_SIZE];
    memset(buffer, 0, MAX_MSG_SIZE);
    int recv_bytes = recv_msg(socket, buffer);
    if (recv_bytes > 0){
        printf("%s", buffer);
    }
    return recv_bytes;
}

/* extras*/
int send_help(int socket)
{
    char msg[] = "HELP";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int send_syst(int socket)
{
    char msg[] = "SYST";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int send_stat(int socket)
{
    char msg[] = "STAT";
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

int send_raw(int socket, char *msg)
{
    return send_msg(socket, (uchar *)msg, strlen(msg) + 1);
}

float transfer_rate(int size, struct timeval *after, struct timeval *before)
{
    long int delta = after->tv_usec + 1000000 * after->tv_sec - (before->tv_usec + 1000000 * before->tv_sec);
    double seconds = delta /(double)1000000;
    return (size/(double)1024)/seconds;
}
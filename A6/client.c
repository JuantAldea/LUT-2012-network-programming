/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                   client.c                  #
###############################################
*/

#include "client.h"

int client_CLI(char **msg)
{
    int bytes_on_stdin;
    ioctl(STDIN_FILENO, FIONREAD, &bytes_on_stdin);
    char *buffer = NULL;
    buffer = (char*)malloc(sizeof(char) * (bytes_on_stdin + 1));
   
    if (fgets(buffer, bytes_on_stdin, stdin) == NULL){
        printf("Error while reading from STDIN\n");
    }

    fgetc(stdin);//read newline
    if(!strlen(buffer)){
        *msg = (char*)malloc(sizeof(char));
        *msg[0] = '\0';
        return UNKOWN_COMMAND_CODE;
    }

    int command_code = UNKOWN_COMMAND_CODE;
    if(!strncmp(buffer, OPEN_COMMAND, strlen(OPEN_COMMAND))){
        command_code = OPEN_COMMAND_CODE;
    }else if(!strncmp(buffer, CD_COMMAND, strlen(CD_COMMAND))){
        command_code = CD_COMMAND_CODE;
        //READ MORE THINGS
    }else if(!strncmp(buffer, CLOSE_COMMAND, strlen(CLOSE_COMMAND))){
        command_code = CLOSE_COMMAND_CODE;
    }else if(!strncmp(buffer, ACTIVE_COMMAND, strlen(ACTIVE_COMMAND))){
        command_code = ACTIVE_COMMAND_CODE;
    }else if(!strncmp(buffer, PASSIVE_COMMAND, strlen(PASSIVE_COMMAND))){
        command_code = PASSIVE_COMMAND_CODE;
    }else if(!strncmp(buffer, GET_COMMAND, strlen(GET_COMMAND))){
        command_code = GET_COMMAND_CODE;
        //READ MORE THINGS
    }else if(!strncmp(buffer, PUT_COMMAND, strlen(PUT_COMMAND))){
        command_code = PUT_COMMAND_CODE;
        //READ MORE THINGS
    }else if(!strncmp(buffer, LS_COMMAND, strlen(LS_COMMAND))){
        command_code = LS_COMMAND_CODE;
    }else if(!strncmp(buffer, BINARY_COMMAND, strlen(BINARY_COMMAND))){
        command_code = BINARY_COMMAND_CODE;
    }else if(!strncmp(buffer, QUIT_COMMAND, strlen(QUIT_COMMAND))){
        command_code = QUIT_COMMAND_CODE;
    }
    free(buffer);
    return command_code;
}

int client(char *address, char *port)
{
    fd_set ready_set;
    FD_ZERO(&ready_set);
    int not_connected = 1;
    int socket_descriptor = -1;
    while (not_connected){
        FD_SET(STDIN_FILENO, &ready_set);
        if(select(STDIN_FILENO + 1, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            return EXIT_FAILURE;
        }
        if (FD_ISSET(STDIN_FILENO, &ready_set)){
            char *message = NULL;
            int command = client_CLI(&message);
            switch(command){
                case OPEN_COMMAND_CODE:
                    socket_descriptor = prepare_connection(address, port);
                    char buffer[MAX_MSG_SIZE];
                    memset(buffer, '\0', MAX_MSG_SIZE);
                    printf("[RIGTH AFTER prepare_connection]\n");
                    //printf("prepare_connection FIN\n");
                    
                    //printf("[SENT]\n"); 
                    not_connected = 0;
                    break;
                case QUIT_COMMAND_CODE:
                    printf("[QUITTING...]\n");
                    return 0;
                    break;
                default:
                    break;
            }
            //free(message);
        }
    }


    int number_of_fds = socket_descriptor + 1;
    int running = 1;
    //we are connected, now we will listen to stdin and the connection socket
    while(running){
        FD_SET(STDIN_FILENO, &ready_set);
        FD_SET(socket_descriptor, &ready_set);
        //select descriptors with activity
        if(select(number_of_fds, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            return EXIT_FAILURE;
        }
        // //activity in stdin
        // if (FD_ISSET(STDIN_FILENO, &ready_set)){
        //     //parse the command
        //     char *message = NULL;
        //     int command = client_CLI(&message);
        //     //and do what you have to do
        //     //switch(command){
        //     //}
        //     free(message);
        // }

        //activity in the socket
        if(FD_ISSET(socket_descriptor, &ready_set)){
            
            char buffer[2048]; 
            memset(buffer, '\0', 2048);
            printf("[SLEEPING ON RECV]\n");
            int recv_bytes = recv_msg(socket_descriptor, buffer);
            printf("[SENDING ANONYMOUS LOGIN]->%d<-\n", send_anonymous_login(socket_descriptor));
            printf("[SENT]\n");
            if (recv_bytes <= 0){
                printf("Connection to the server lost\n");
                running = 0;
            }else{
                //dump_msg((uchar*)buffer, recv_bytes);
            }
        }
    }
    close(socket_descriptor);
    return EXIT_SUCCESS;
}

//returns the socket descriptor of the
int prepare_connection(char *address, char *port)
{
    struct addrinfo hints;
    //fill all the fields with zero, just to be sure. Actually it crashed without this.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = IPPROTO_TCP;

    int error = 0;
    struct addrinfo *res = NULL;
    if ((error = getaddrinfo(address, port, &hints, &res)) < 0){
        perror(gai_strerror(error));
        freeaddrinfo(res);
        printf("[ERROR ON prepare_connection]\n");
        return EXIT_FAILURE;
    }

    int socket_descriptor = -1;
    struct addrinfo *ptr;
    //loop through the address looking for one that works
    for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        if ((socket_descriptor = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0) {
            printf("socket failed %s\n", strerror(errno));
            continue;
        }

        int option_value = 1;
        if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) < 0){
            printf("setsockopt failed %s\n", strerror(errno));
            close(socket_descriptor);
            continue;
        }

        if(connect(socket_descriptor, ptr->ai_addr, ptr->ai_addrlen) < 0){
            printf("Connect failed: %s\n", strerror(errno));
            close(socket_descriptor);
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    
    //if no addr is found, we have a problem
    if (ptr == NULL){
        printf("[ERROR ON prepare_connection]\n");
        return -1;
    }
    
    return socket_descriptor;
}

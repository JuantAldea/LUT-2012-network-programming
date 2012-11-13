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
    }else if(!strncmp(buffer, LOGIN_ANONYMOUS, strlen(LOGIN_ANONYMOUS))){
        command_code = LOGIN_ANONYMOUS_COMMAND_CODE;
    }else if(!strncmp(buffer, LOGIN, strlen(LOGIN))){
        command_code = LOGIN_COMMAND_CODE;
    }
    free(buffer);
    return command_code;
}

int client(char *address, char *port)
{
    enum states client_state = NOT_CONNECTED;
    enum transfer_modes transfer_mode = PASSIVE;

    fd_set ready_set;
    FD_ZERO(&ready_set);
    int socket_descriptor = -1;
    int socket_transfer = -1;
    int running = 1;

    while (running){
        //the program will only allow one transfer
        int max_fd = (socket_descriptor > socket_transfer) ? socket_descriptor : socket_transfer;
        max_fd = (max_fd > STDIN_FILENO) ? max_fd : STDIN_FILENO;
        FD_SET(STDIN_FILENO, &ready_set);
        if (socket_descriptor > 0){
            FD_SET(socket_descriptor, &ready_set);
        }

        if(socket_transfer > 0){
            FD_SET(socket_transfer, &ready_set);
        }

        if(select(max_fd + 1, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            return EXIT_FAILURE;
        }

        if (FD_ISSET(STDIN_FILENO, &ready_set)){
            char *message = NULL;
            int command = client_CLI(&message);
            switch(command){
                case OPEN_COMMAND_CODE:
                    if(client_state == NOT_CONNECTED){
                        socket_descriptor = prepare_connection(address, port);
                        if (!read_greetings(socket_descriptor)){
                            running = 0;
                        }else{
                            client_state = CONNECTED;
                        }
                    }else if (client_state > NOT_CONNECTED){
                        printf("[COMMAND IGNORED]: Already connected.\n");
                    }
                    break;
                case LOGIN_ANONYMOUS_COMMAND_CODE:
                    if (client_state == CONNECTED){
                        if (send_anonymous_login(socket_descriptor)){
                            client_state = LOGGED_IN;
                        }else{
                            running = 0;
                        }
                    }else if (client_state > CONNECTED){
                        printf("[COMMAND IGNORED]: Already logged in.\n");
                    }
                    break;
                case LOGIN_COMMAND_CODE:
                    if(client_state == CONNECTED){
                        char *username, *password;
                        ask_login(&username, &password);
                        if (send_login(socket_descriptor, username, password)){
                            client_state = LOGGED_IN;
                        }else{
                            running = 0;
                        }
                    }else if (client_state > CONNECTED){
                        printf("[COMMAND IGNORED]: Already logged in.\n");
                    }
                    break;
                case QUIT_COMMAND_CODE:
                    if (send_quit(socket_descriptor)){
                        printf("[SCHEDULING QUIT]\n");
                    }
                    break;
                default:
                    break;
            }
            //free(message);
        }

        if(FD_ISSET(socket_descriptor, &ready_set)){
            char buffer[2048];
            memset(buffer, '\0', 2048);
            int recv_bytes = recv_msg(socket_descriptor, buffer);
            if (recv_bytes <= 0){
                printf("Connection to the server lost\n");
                running = 0;
            }else{
                if(!strncmp(buffer, "221", 3)){
                    printf("[DISCONNECTED FORM SERVER]: %s\n", buffer);
                    running = 0;
                }
                dump_msg((uchar*)buffer, recv_bytes);
            }
        }
    }


/*
    int number_of_fds = socket_descriptor + 1;
    int running = 1;
    //we are connected, now we will listen to stdin and the connection socket
    while(running){
        FD_SET(STDIN_FILENO, &ready_set);
        FD_SET(socket_descriptor, &ready_set);
        printf("[SLEEPING ON SELECT]\n");
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
            char buffeytes <= 0){
                printf("Connection to the server lost\n");
                running = 0;
            }else{
                //dump_msg((uchar*)buffer, recv_bytes);
            }
        }
    }r[2048];
            memset(buffer, '\0', 2048);
            printf("[SLEEPING ON RECV]\n");
            int recv_bytes = recv_msg(socket_descriptor, buffer);
            if (recv_bytes <= 0){
                printf("Connection to the server lost\n");
                running = 0;
            }else{
                //dump_msg((uchar*)buffer, recv_bytes);
            }
        }
    }
    */
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


int read_greetings(int socket)
{
    char buffer[MAX_MSG_SIZE];
    memset(buffer, '\0', MAX_MSG_SIZE);
    int recv_bytes = recv_msg(socket, buffer);
    if (recv_bytes <= 0){
        printf("[ERROR]: There is some problem with the server.\n");
        return EXIT_FAILURE;
    }
    if(!strncmp(buffer, "220", 3)){
        printf("[CONNECTED TO THE SERVER]: %s", buffer);
    }else{
        printf("[PROBLEM]: %s", buffer);
    }
    return recv_bytes;
}

void ask_login(char **username, char **password)
{
    int bytes_on_stdin;
    fd_set ready_set;

    printf("Enter username:\n");

    FD_ZERO(&ready_set);
    FD_SET(STDIN_FILENO, &ready_set);
    if(select(STDIN_FILENO + 1, &ready_set, NULL, NULL, NULL) >= 0) {
        ioctl(STDIN_FILENO, FIONREAD, &bytes_on_stdin);
        printf("Bytes %d\n", bytes_on_stdin);
        *username = (char*)malloc(sizeof(char) * (bytes_on_stdin + 1));
        memset(*username, '\0', bytes_on_stdin + 1);
        if(fgets(*username, bytes_on_stdin, stdin) == NULL){
            printf("Error while reading from STDIN\n");
        }
        fgetc(stdin);//read newline
    }
    printf("%s\n", *username);

    printf("Enter password:\n");

    struct termios t;
    struct termios t2;
    ioctl(0, TCGETS, &t);
    ioctl(0, TCGETS, &t2);
    t.c_lflag &= ~ECHO;
    ioctl(0, TCSETS, &t);

    FD_ZERO(&ready_set);
    FD_SET(STDIN_FILENO, &ready_set);
    if(select(STDIN_FILENO + 1, &ready_set, NULL, NULL, NULL) >= 0) {
        ioctl(STDIN_FILENO, FIONREAD, &bytes_on_stdin);
        *password = (char*)malloc(sizeof(char) * (bytes_on_stdin + 1));
        memset(*password, '\0', bytes_on_stdin + 1);
        if(fgets(*password, bytes_on_stdin, stdin) == NULL){
            printf("Error while reading from STDIN\n");
        }
        fgetc(stdin);//read newline
    }
    ioctl(0, TCSETS, &t2);
}
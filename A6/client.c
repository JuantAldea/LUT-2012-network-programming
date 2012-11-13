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
    printf("BYTES: %d\n", bytes_on_stdin);
    char *buffer = (char*)malloc(sizeof(char) * (bytes_on_stdin));
    //char buffer[3];
    memset(buffer, '\0', bytes_on_stdin);

    if (fgets(buffer, bytes_on_stdin, stdin) == NULL){
        printf("Error reading from STDIN\n");
        return UNKOWN_COMMAND_CODE;
    }
    dump_msg((uchar*)buffer, bytes_on_stdin);
    for (int i = 0; i<100; i++){
        if (buffer[i]=='\0'){
            printf("Longitud %d\n", i);
            break;
        }
    }
    //int a = strlen(buffer);
    //a = strlen(buffer);
    //printf("%d\n", a);
    //printf("kkkkkkkkkkk%c %d\n", buffer[0], buffer[1]=='\0');

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
        int path_length = strlen(buffer) - (strlen(CD_COMMAND) + 1) + 1;
        printf("asdadad:%d\n", path_length);
        if(path_length > 0){
            *msg = malloc(sizeof(char) * path_length);
            memcpy(*msg, &buffer[strlen(CD_COMMAND) + 1], path_length);
        }else{
            printf("[ERROR]: Invalid path\n");
        }
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
    }else if(!strncmp(buffer, HELP, strlen(HELP))){
        command_code = HELP_COMMAND_CODE;
    }
    //free(buffer);
    return command_code;
}

int client(char *address, char *port)
{
    enum states client_state = NOT_CONNECTED;
    enum transfer_modes transfer_mode = PASSIVE;

    fd_set ready_set;
    FD_ZERO(&ready_set);
    int socket_control = -1;
    int socket_transfer = -1;
    int running = 1;

    while (running){
        //the program will only allow one transfer
        int max_fd = (socket_control > socket_transfer) ? socket_control : socket_transfer;
        max_fd = (max_fd > STDIN_FILENO) ? max_fd : STDIN_FILENO;
        FD_SET(STDIN_FILENO, &ready_set);
        if (socket_control > 0){
            FD_SET(socket_control, &ready_set);
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
                        socket_control = prepare_connection(address, port);
                        printf("[CONNECTING...]\n");
                    }else if (client_state > NOT_CONNECTED){
                        printf("[COMMAND IGNORED]: Already connected.\n");
                    }
                    break;
                case LOGIN_ANONYMOUS_COMMAND_CODE:
                    if (client_state == CONNECTED){
                        send_anonymous_login(socket_control);
                    }else if (client_state > CONNECTED){
                        printf("[COMMAND IGNORED]: Already logged in.\n");
                    }else{
                        printf("[COMMAND IGNORED: Connect first\n");
                    }
                    break;
                case LOGIN_COMMAND_CODE:
                    if(client_state == CONNECTED){
                        char *username, *password;
                        ask_login(&username, &password);
                        send_login(socket_control, username, password);
                        free(username);
                        free(password);
                    }else if (client_state > CONNECTED){
                        printf("[COMMAND IGNORED]: Already logged in.\n");
                    }
                    break;
                case QUIT_COMMAND_CODE:
                    if (send_quit(socket_control)){
                        printf("[SCHEDULING QUIT]\n");
                    }
                    break;
                case PASSIVE_COMMAND_CODE:
                    if(!enter_passive_mode(socket_control)){
                        printf("[ERROR]: Quitting...");
                        running = 0;
                    }
                    break;
                case LS_COMMAND_CODE:
                    if(!send_list(socket_control)){
                        printf("[ERROR LS]: Quitting...");
                    }
                    break;
                case CD_COMMAND_CODE:
                    send_cd(socket_control, message);
                    break;
                default:
                    printf("[ERROR] Unknow command\n");
                    break;
            }
            if(message != NULL){
                free(message);
                message = NULL;
            }
        }

        if(FD_ISSET(socket_control, &ready_set)){
            char buffer[2048];
            memset(buffer, '\0', 2048);
            int recv_bytes = recv_msg(socket_control, buffer);
            if (recv_bytes <= 0){
                printf("Connection to the server lost\n");
                running = 0;
            }else{
                if(!strncmp(buffer, "220", 3)){
                    printf("[CONNECTED TO THE SERVER]: %s", buffer);
                    client_state = CONNECTED;
                }else if(!strncmp(buffer, "221", 3)){
                    printf("[DISCONNECTED FORM SERVER]: %s\n", buffer);
                    running = 0;
                    client_state = NOT_CONNECTED;
                }else if(!strncmp(buffer, "421", 3)){
                    running = 0;
                    printf("[DISCONNECTED]: %s\n", buffer);
                    client_state = NOT_CONNECTED;
                }else if(!strncmp(buffer, "230", 3)){
                    printf("[LOGGED IN]: %s\n", buffer);
                    client_state = LOGGED_IN;
                }else if(!strncmp(buffer, "227", 3)){
                    transfer_mode = PASSIVE;
                    printf("[ENTERING PASSIVE MODE]: %s\n", buffer);
                    char *server_info = strchr(buffer, '(');
                    char h1[3], h2[3], h3[3], h4[3], p1[3], p2[3];
                    sscanf(server_info,"(%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", h1, h2, h3, h4, p1, p2);
                    char ip[16];
                    sprintf(ip, "%s.%s.%s.%s", h1, h2, h3, h4);
                    char port[6];
                    sprintf(port, "%d", atoi(p1) * 256 + atoi(p2));
                    socket_transfer = prepare_connection(ip, port);
                }else if(!strncmp(buffer, "250", 3)){
                    printf("[SUCESS]: %s\n", buffer);
                }else{
                    dump_msg((uchar*)buffer, recv_bytes);
                }
            }
        }

        if(FD_ISSET(socket_transfer, &ready_set)){
            int bytes_availables;
            ioctl(socket_transfer, FIONREAD, &bytes_availables);
            if(bytes_availables == 0){
                close(socket_transfer);
                socket_transfer = -1;
                continue;
            }
            printf("%d\n", bytes_availables);
            char *buffer = (char*)malloc(sizeof(char) * bytes_availables);
            int recv_bytes = recv_msg(socket_transfer, buffer);
            printf("=============================================%d\n", recv_bytes);
            dump_msg((uchar*)buffer, bytes_availables);
            free(buffer);
        }
    }


/*
    int number_of_fds = socket_control + 1;
    int running = 1;
    //we are connected, now we will listen to stdin and the connection socket
    while(running){
        FD_SET(STDIN_FILENO, &ready_set);
        FD_SET(socket_control, &ready_set);
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
        if(FD_ISSET(socket_control, &ready_set)){
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
            int recv_bytes = recv_msg(socket_control, buffer);
            if (recv_bytes <= 0){
                printf("Connection to the server lost\n");
                running = 0;
            }else{
                //dump_msg((uchar*)buffer, recv_bytes);
            }
        }
    }
    */
    close(socket_control);
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

    int socket_control = -1;
    struct addrinfo *ptr;
    //loop through the address looking for one that works
    for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        if ((socket_control = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0) {
            printf("socket failed %s\n", strerror(errno));
            continue;
        }

        int option_value = 1;
        if (setsockopt(socket_control, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) < 0){
            printf("setsockopt failed %s\n", strerror(errno));
            close(socket_control);
            continue;
        }

        if(connect(socket_control, ptr->ai_addr, ptr->ai_addrlen) < 0){
            printf("Connect failed: %s\n", strerror(errno));
            close(socket_control);
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
    return socket_control;
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
        *username = (char*)malloc(sizeof(char) * (bytes_on_stdin + 1));
        memset(*username, '\0', bytes_on_stdin + 1);
        if(fgets(*username, bytes_on_stdin, stdin) == NULL){
            printf("Error while reading from STDIN\n");
        }
        fgetc(stdin);//read newline
    }
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
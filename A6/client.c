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
    char *buffer = (char*)malloc(sizeof(char) * (bytes_on_stdin));
    //char buffer[3];
    memset(buffer, '\0', bytes_on_stdin);

    if (fgets(buffer, bytes_on_stdin, stdin) == NULL){
        printf("Error reading from STDIN\n");
        return UNKOWN_COMMAND_CODE;
    }

    fgetc(stdin);//read newline
    if(!strlen(buffer)){
        free(buffer);
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
        int path_length = strlen(buffer) - (strlen(GET_COMMAND) + 1) + 1;
        if(path_length > 0){
            *msg = malloc(sizeof(char) * path_length);
            memcpy(*msg, &buffer[strlen(GET_COMMAND) + 1], path_length);
        }else{
            printf("[ERROR]: Invalid path\n");
        }
    }else if(!strncmp(buffer, PUT_COMMAND, strlen(PUT_COMMAND))){
        command_code = PUT_COMMAND_CODE;
        int path_length = strlen(buffer) - (strlen(PUT_COMMAND) + 1) + 1;
        if(path_length > 0){
            *msg = malloc(sizeof(char) * path_length);
            memcpy(*msg, &buffer[strlen(PUT_COMMAND) + 1], path_length);
        }else{
            printf("[ERROR]: Invalid path\n");
        }
    }else if(!strncmp(buffer, LS_COMMAND, strlen(LS_COMMAND))){
        command_code = LS_COMMAND_CODE;
    }else if(!strncmp(buffer, BINARY_COMMAND, strlen(BINARY_COMMAND))){
        command_code = BINARY_COMMAND_CODE;
    }else if(!strncmp(buffer, QUIT_COMMAND, strlen(QUIT_COMMAND))){
        command_code = QUIT_COMMAND_CODE;
    }else if(!strncmp(buffer, HELP, strlen(HELP))){
        command_code = HELP_COMMAND_CODE;
    }
    free(buffer);
    return command_code;
}

int client(char *address, char *port)
{
    char *username = NULL;
    char *password = NULL;
    char *remote_path = NULL;
    char *local_path = NULL;
    enum states client_state = NOT_CONNECTED;
    enum transfer_modes transfer_mode = PASSIVE;
    enum transfer_types transfer_pending = NONE;

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
                        printf("[CONNECTING]\n");
                    }else if (client_state > NOT_CONNECTED){
                        printf("[COMMAND IGNORED]: Already connected.\n");
                    }
                    break;
                case CLOSE_COMMAND_CODE:
                    if (client_state > NOT_CONNECTED){
                        if (send_quit(socket_control)){
                            close(socket_control);
                            socket_control = -1;
                            client_state = NOT_CONNECTED;
                        }
                    }
                    break;
                case QUIT_COMMAND_CODE:
                    if (client_state > NOT_CONNECTED){
                        if (send_quit(socket_control)){
                            printf("[SCHEDULING QUIT]\n");
                        }
                    }else{
                        running = 0;
                    }
                    break;
                case PASSIVE_COMMAND_CODE:
                    transfer_mode = PASSIVE;
                    if(!enter_passive_mode(socket_control)){
                        printf("[ERROR]: Quitting...");
                        running = 0;
                    }
                    break;
                case LS_COMMAND_CODE:
                    if(transfer_mode == PASSIVE){
                        if(!enter_passive_mode(socket_control)){
                            printf("[ERROR]: Quitting...");
                            running = 0;
                        }
                    }else{

                    }

                    if(send_list(socket_control)){
                        transfer_pending = LS;
                    }else{
                        printf("[ERROR LS]: Quitting...");
                    }

                    break;
                case CD_COMMAND_CODE:
                    send_cd(socket_control, message);
                    break;
                case PUT_COMMAND_CODE:
                {
                    char *filename = strrchr(message, '/');
                    if (filename == NULL){
                        filename = message;
                    }else if(filename[1] == '\0'){//last character is a /, name is invalid
                        printf("[ERROR]: The path is a folder");
                        filename = NULL;
                    }else{
                        filename++;
                    }

                    if (remote_path != NULL){
                        free(remote_path);
                    }
                    if(local_path != NULL){
                        free(local_path);
                    }

                    local_path  = strdup(message);
                    remote_path = strdup(filename);

                    if (filename != NULL){
                        printf("MIERDA\n");
                        if (transfer_mode == PASSIVE){
                            if(!enter_passive_mode(socket_control)){
                                printf("[ERROR]: Quitting...");
                                running = 0;
                            }
                        }else{

                        }
                        send_put(socket_control, filename);
                        transfer_pending = SEND;
                    }else{

                    }
                }
                    break;
                case GET_COMMAND_CODE:
                {

                    char *filename = strrchr(message, '/');
                    if (filename == NULL){
                        filename = message;
                    }else if(filename[1] == '\0'){//last character is a /, name is invalid
                        printf("[ERROR]: The path is a folder");
                        filename = NULL;
                    }else{
                        filename++;
                    }

                    if (remote_path != NULL){
                        free(remote_path);
                    }
                    if(local_path != NULL){
                        free(local_path);
                    }
                    remote_path = strdup(message);
                    local_path  = strdup(filename);

                    if (filename != NULL){
                        if (transfer_mode == PASSIVE){
                            if(!enter_passive_mode(socket_control)){
                                printf("[ERROR]: Quitting...");
                                running = 0;
                            }
                        }else{

                        }
                        send_get(socket_control, remote_path);
                        transfer_pending = RECV;
                    }
                }
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
                    client_state = CONNECTED;
                    printf("[CONNECTED TO THE SERVER]: %s", buffer);
                    ask_login(&username, &password);
                    printf("[SENDING USERNAME...]\n");
                    send_username(socket_control, username);
                }else if(!strncmp(buffer, "331", 3)){
                    printf("[USERNAME ACCEPTED]\n");
                    printf("[SENDING PASSWORD...]\n");
                    send_password(socket_control, password);
                }else if(!strncmp(buffer, "221", 3)){
                    printf("[DISCONNECTED FORM SERVER]\n");
                    close(socket_control);
                    socket_control = -1;
                    running = 0;
                    client_state = NOT_CONNECTED;
                }else if(!strncmp(buffer, "421", 3)){
                    running = 0;
                    printf("[DISCONNECTED]: %s\n", buffer);
                    client_state = NOT_CONNECTED;
                }else if(!strncmp(buffer, "230", 3)){
                    printf("[LOGGED IN]\n");
                    client_state = LOGGED_IN;
                }else if(!strncmp(buffer, "227", 3)){;
                    printf("[ENTERING PASSIVE MODE]\n");
                    char *server_info = strchr(buffer, '(');
                    char h1[3], h2[3], h3[3], h4[3], p1[3], p2[3];
                    sscanf(server_info,"(%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", h1, h2, h3, h4, p1, p2);
                    char ip[16];
                    sprintf(ip, "%s.%s.%s.%s", h1, h2, h3, h4);
                    char port[6];
                    sprintf(port, "%d", atoi(p1) * 256 + atoi(p2));
                    socket_transfer = prepare_connection(ip, port);
                }else if(!strncmp(buffer, "250", 3)){
                    printf("[WORKING DIR CHANGED]\n");
                }else if(!strncmp(buffer, "150", 3)){
                    printf("[DATA] Begin of transfer.\n");
                    if (transfer_pending == SEND){
                        printf("alskdjakljdakljdak\n");
                        send_file(socket_transfer, local_path);
                    }

                }else if(!strncmp(buffer, "226", 3)){
                    printf("[DATA] End of transfer.\n");
                    transfer_pending = NONE;
                    if (socket_transfer != -1){
                        close(socket_transfer);
                        socket_transfer = -1;
                    }
                }else if(!strncmp(buffer, "530", 3)){
                    printf("[ERROR] Login incorrect.\n");
                }else{
                    dump_msg((uchar*)buffer, recv_bytes);
                }
            }
        }

        if(FD_ISSET(socket_transfer, &ready_set)){
            if(transfer_pending == LS){
                int bytes_availables;
                ioctl(socket_transfer, FIONREAD, &bytes_availables);
                char *buffer = (char*)malloc(sizeof(char) * (bytes_availables + 1));
                memset(buffer, '\0', sizeof(char) * (bytes_availables + 1));
                int recv_bytes = recv_msg(socket_transfer, buffer);
                if(recv_bytes > 0){
                    printf("%s\n", buffer);
                }
                free(buffer);
            }else if(transfer_pending == RECV){
                recv_file(socket_transfer, local_path);
            }
            close(socket_transfer);
            socket_transfer = -1;
            transfer_pending = NONE;
        }
    }
    free(username);
    free(password);
    free(local_path);
    free(remote_path);
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


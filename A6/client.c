/*
###############################################
#        CT30A5001 - Network Programming      #
#          Assignment 6: FTP Client           #
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
    memset(buffer, '\0', bytes_on_stdin);

    if (fgets(buffer, bytes_on_stdin, stdin) == NULL){
        printf("EOF readed: quit\n");
        free(buffer);
        return EOF_COMMAND_CODE;
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
    }else if(!strncmp(buffer, HELP_COMMAND, strlen(HELP_COMMAND))){
        command_code = HELP_COMMAND_CODE;
    }else if(!strncmp(buffer, SYST_COMMAND, strlen(SYST_COMMAND))){
        command_code = SYST_COMMAND_CODE;
    }else if(!strncmp(buffer, STATUS_COMMAND, strlen(STATUS_COMMAND))){
        command_code = STATUS_COMMAND_CODE;
    }else if(!strncmp(buffer, RAW_COMMAND, strlen(RAW_COMMAND))){
        command_code = RAW_COMMAND_CODE;
        int command_size = strlen(&buffer[4]);
        *msg = malloc(sizeof(char) * (command_size +1));
        memset(*msg, 0, sizeof(char) * (command_size +1));
        memcpy(*msg, &buffer[4], sizeof(char) * (command_size +1));
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
    int socket_active = -1;
    int running = 1;
    int quit = 0;//if quit command => quit = 1, if close command => quit = 0
    while (running){
        int max_fd = (socket_control > socket_transfer) ? socket_control : socket_transfer;
        max_fd = (max_fd > STDIN_FILENO) ? max_fd : STDIN_FILENO;
        max_fd = (max_fd > socket_active) ? max_fd : socket_active;
        FD_SET(STDIN_FILENO, &ready_set);
        if(socket_control > 0){
            FD_SET(socket_control, &ready_set);
        }

        if(socket_transfer > 0){
            FD_SET(socket_transfer, &ready_set);
        }

        if(socket_active > 0){
            FD_SET(socket_active, &ready_set);
        }

        if(select(max_fd + 1, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            return EXIT_FAILURE;
        }

        if (FD_ISSET(STDIN_FILENO, &ready_set)){
            char *message = NULL;
            int command = client_CLI(&message);
            switch(command){
                case EOF_COMMAND_CODE:
                    reset_client(&socket_control, &socket_active, &socket_transfer, &client_state);
                    running = 0;
                    break;
                case OPEN_COMMAND_CODE:
                    if(client_state < LOGGED_IN){
                        socket_control = prepare_connection(address, port);
                        printf("[CONNECTING]\n");
                    }else{
                        printf("[COMMAND IGNORED]: Already connected.\n");
                    }
                    break;
                case CLOSE_COMMAND_CODE:
                    if (client_state > NOT_CONNECTED){
                        send_quit(socket_control);
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
                    quit = 1;
                    break;
                case PASSIVE_COMMAND_CODE:
                    transfer_mode = PASSIVE;
                    printf("[TRANSFER MODE]: Passive\n");
                    break;
                case ACTIVE_COMMAND_CODE:
                    transfer_mode = ACTIVE;
                    printf("[TRANSFER MODE]: Active\n");
                    break;
                case LS_COMMAND_CODE:
                    if (client_state == LOGGED_IN){
                        send_mode(socket_control, &socket_active, transfer_mode);
                        int error;
                        if((error = send_list(socket_control)) > 0){
                            transfer_pending = LS;
                        }else{
                            printf("[ERROR LS]: Quitting...");
                        }
                    }else{
                        printf("[ERROR]: Log in first\n");
                    }
                    break;
                case CD_COMMAND_CODE:
                    if (client_state == LOGGED_IN){
                        send_cd(socket_control, message);
                    }else{
                        printf("[ERROR]: Log in first\n");
                    }
                    break;
                case PUT_COMMAND_CODE:
                    if (client_state == LOGGED_IN){

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
                            send_mode(socket_control, &socket_active, transfer_mode);
                            send_put(socket_control, filename);
                            transfer_pending = SEND;
                        }else{

                        }
                    }else{
                        printf("[ERROR]: Log in first\n");
                    }
                    break;
                case GET_COMMAND_CODE:
                    if (client_state == LOGGED_IN){

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
                            send_mode(socket_control, &socket_active, transfer_mode);
                            send_get(socket_control, remote_path);
                            transfer_pending = RECV;
                        }
                    }else{
                        printf("[ERROR]: Login first\n");
                    }
                    break;
                case HELP_COMMAND_CODE:
                    if (client_state > CONNECTED){
                        printf("[HELP]: You can use all commands recognized by the server using the \"raw\" command.\n");
                        send_help(socket_control);
                    }
                    break;
                case SYST_COMMAND_CODE:
                    if (client_state > CONNECTED){
                        send_syst(socket_control);
                    }
                    break;
                case STATUS_COMMAND_CODE:
                    if (client_state > CONNECTED){
                        send_stat(socket_control);
                    }
                case RAW_COMMAND_CODE:
                    if(client_state > NOT_CONNECTED){
                        send_raw(socket_control, message);
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
        }else if(FD_ISSET(socket_control, &ready_set)){
            char buffer[MAX_MSG_SIZE];
            memset(buffer, '\0', MAX_MSG_SIZE);
            int recv_bytes = recv_msg(socket_control, buffer);
            if (recv_bytes <= 0){
                printf("Connection to the server lost\n");
                running = 0;
            }else{
                if(!strncmp(buffer, "220", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        client_state = CONNECTED;
                        if (username != NULL){
                            free(username);
                            username = NULL;
                        }
                        if (password != NULL){
                            free(password);
                            password = NULL;
                        }
                        ask_login(&username, &password);
                        send_username(socket_control, username);
                    }
                }else if(!strncmp(buffer, "331", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        printf("[USERNAME ACCEPTED]\n");
                        printf("[SENDING PASSWORD...]\n");
                        send_password(socket_control, password);
                        send_binary(socket_control);
                    }
                }else if(!strncmp(buffer, "221", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        printf("[DISCONNECTED FORM SERVER]\n");
                        reset_client(&socket_control, &socket_active, &socket_transfer, &client_state);
                        if(quit){
                            running = 0;
                        }
                    }
                }else if(!strncmp(buffer, "421", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        running = 0;
                        printf("[DISCONNECTED]: %s\n", buffer);
                        client_state = NOT_CONNECTED;
                    }
                }else if(!strncmp(buffer, "230", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        printf("[LOGGED IN]\n");
                        client_state = LOGGED_IN;
                    }
                }else if(!strncmp(buffer, "227", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        char *server_info = strchr(buffer, '(');
                        char h1[3], h2[3], h3[3], h4[3], p1[3], p2[3];
                        sscanf(server_info,"(%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", h1, h2, h3, h4, p1, p2);
                        char ip[16];
                        sprintf(ip, "%s.%s.%s.%s", h1, h2, h3, h4);
                        char port[6];
                        sprintf(port, "%d", atoi(p1) * 256 + atoi(p2));
                        if (socket_transfer != -1){
                            close(socket_transfer);
                            socket_transfer = -1;
                        }
                        socket_transfer = prepare_connection(ip, port);
                    }
                }else if(!strncmp(buffer, "150", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        if (transfer_pending == SEND){
                            send_file(socket_transfer, local_path);
                            close(socket_transfer);
                            socket_transfer = -1;
                            transfer_pending = NONE;
                        }
                    }
                }else if(!strncmp(buffer, "550", 3)){
                    printf("%s", buffer);
                    if(buffer[3] == ' '){
                        transfer_pending = NONE;
                    }
                }else{
                    printf("%s", buffer);
                }
            }
        }else if(FD_ISSET(socket_transfer, &ready_set)){
            if(transfer_pending == LS){
                if (recv_ls(socket_transfer) <= 0){
                    transfer_pending = NONE;
                    if (socket_transfer != -1){
                        close(socket_transfer);
                        socket_transfer = -1;
                    }
                }
            }else if(transfer_pending == RECV){
                recv_file(socket_transfer, local_path);
                transfer_pending = NONE;
                if (socket_transfer != -1){
                    close(socket_transfer);
                    socket_transfer = -1;
                }
            }else{

            }
        }else if(FD_ISSET(socket_active, &ready_set)){
            if (socket_transfer != -1){
                close(socket_transfer);
                socket_transfer = -1;
            }
            struct sockaddr addr;
            socklen_t addr_size = sizeof(struct sockaddr);
            socket_transfer = accept(socket_active, &addr, &addr_size);
        }
    }
    free(username);
    free(password);
    free(local_path);
    free(remote_path);
    reset_client(&socket_control, &socket_active, &socket_transfer, &client_state);
    return EXIT_SUCCESS;
}

int prepare_connection(char *address, char *port)
{
    struct addrinfo hints;
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
        printf("[ERROR ON connect_in_passive_mode]\n");
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
        printf("[ERROR ON connect_in_passive_mode]\n");
        return -1;
    }
    return socket_control;
}

int open_active_mode_server(int control_socket, int ip[4], int port[2])
{
    struct sockaddr_in data_addr;
    socklen_t len = sizeof(data_addr);
    getsockname(control_socket, (struct sockaddr *)&data_addr, &len);
    //printf("%s\n", inet_ntoa(data_addr.sin_addr));


    data_addr.sin_port = 0; //system will pick a suitable one
    int data_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(data_socket, (struct sockaddr*)&data_addr, sizeof (data_addr)) < 0) {
        perror("bind");
    }
    // if (getsockname(data_socket, (struct sockaddr*)&data_addr, &len) < 0) {
    //     perror("getsockname");
    // }
    if (listen(data_socket, 1) < 0){
        perror("listen");
    }

    if (getsockname(data_socket, (struct sockaddr *)&data_addr, &len) == 0){
        int full_port = ntohs(data_addr.sin_port);
        port[0] = full_port >> 8;
        port[1] = full_port & 0xff;
        char *ip_str = inet_ntoa(data_addr.sin_addr);
        sscanf(ip_str,"%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
        printf("%d %d %d\n", port[0], port[1], full_port);
    }else{
        perror("getsockname");
    }
    return data_socket;
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
    //disable echo
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
    //enable echo
    ioctl(0, TCSETS, &t2);
}

int send_mode(int socket_control, int *socket_active, enum transfer_modes transfer_mode)
{
    if(transfer_mode == PASSIVE){
        if(!enter_passive_mode(socket_control)){
            return 0;
        }
    }else if(transfer_mode == ACTIVE){
        int ip[4], port[2];
        if(*socket_active != -1){
            close(*socket_active);
            *socket_active = -1;
        }
        *socket_active = open_active_mode_server(socket_control, ip, port);
        if(!enter_active_mode(socket_control, ip, port)){
            return 0;
        }
    }
    return 1;
}

void reset_client(int *socket_control, int *socket_active, int *socket_transfer, enum states *client_state)
{
    *client_state = NOT_CONNECTED;
    if(*socket_control != -1){
        close(*socket_control);
        *socket_control = -1;
    }
    if(*socket_transfer != -1){
        close(*socket_transfer);
        *socket_transfer = -1;
    }
    if(*socket_active != -1){
        close(*socket_active);
        *socket_active = -1;
    }
}

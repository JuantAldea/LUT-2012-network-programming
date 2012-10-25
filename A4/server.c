/*
####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                     server.c                     #
####################################################
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server.h"
#include <sys/ioctl.h>
#include "common.h"
#include "server.h"

#define HELLO_PORT 27024
#define HELLO_GROUP "225.0.0.37"

int main(int argc, char **argv)
{
    int optc = -1;
    char *port = NULL;
    char *ip = NULL;
    char *open_port = NULL;
    int port_number, open_port_number;
    while ((optc = getopt(argc, argv, "p:m:o:")) != -1) {
        switch (optc) {
            case 'm':
                ip = optarg;
            break;
            case 'o':
                if (is_number(optarg, 10, &open_port_number)){
                    open_port = optarg;
                }else{
                    printf("Invalid open port number: %s\n", optarg);
                }
            case 'p':
                if (is_number(optarg, 10, &port_number)){
                    port = optarg;
                }else{
                    printf("Invalid port number: %s\n", optarg);
                }
            break;
            case ':':
                printf ("Something?\n");
            break;
            case '?':
                switch(optopt){
                    case 'p':
                        printf("-%c: Missing port.", optopt);
                    break;
                }
            break;
        }
    }

    if (port != NULL && open_port != NULL && ip != NULL){
        return server(port, ip, open_port);
    }else{
        printf("Wrong sintax\n");
        help(argv[0]);
    }
    return 0;
}


int server(char *port, char *multicast_addr, char *multicast_port)
{
    int open_socket = -1;
    int multicast_socket = -1;
    int client_socket = -1;
    struct addrinfo *addr = NULL;
    int running = 1;
    linked_list_t *players = (linked_list_t *) malloc(sizeof(linked_list_t));
    list_init(players);
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int error = 0;
    struct addrinfo *res = NULL;

    if ((error = getaddrinfo(NULL, port, &hints, &res)) < 0){
        printf("Getaddrinfo error: %s\n", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in multicast_group;
    memset(&multicast_group, 0, sizeof(multicast_group));
    multicast_group.sin_family = AF_INET;
    multicast_group.sin_addr.s_addr = inet_addr(HELLO_GROUP);
    multicast_group.sin_port = htons(HELLO_PORT);
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(HELLO_GROUP);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    //loop looking for a addr that works
    for(addr = res; addr != NULL; addr = addr->ai_next) {
        if ((open_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0) {
            printf("socket failed %s\n", strerror(errno));
            continue;
        }

        int option_value = 1;
        if (setsockopt(open_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) < 0){
            printf("setsockopt failed %s\n", strerror(errno));
            close(open_socket);
            continue;
        }
        if (bind(open_socket, addr->ai_addr, addr->ai_addrlen) < 0) {
            printf("Bind failed");
            close(open_socket);
            continue;
        }

        if ((multicast_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0) {
            printf("socket failed %s\n", strerror(errno));
            continue;
        }

        if (setsockopt(multicast_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) < 0){
            printf("setsockopt failed %s\n", strerror(errno));
            close(open_socket);
            continue;
        }

        if (bind(multicast_socket, (struct sockaddr*)&multicast_group, sizeof(multicast_group)) < 0) {
            printf("Bind failed");
            close(open_socket);
            close(multicast_socket);
            continue;
        }

        if(setsockopt(multicast_socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) != 0 ) {
            close(multicast_socket);
            close(open_socket);
            printf("Failed joining to the group: %s\n", strerror(errno));
            continue;
        }

       // u_char loop = 0;
        //setsockopt(multicast_socket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
        if ((client_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0) {
            close(multicast_socket);
            close(open_socket);
            printf("socket failed %s\n", strerror(errno));
            continue;
        }
        if (setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) < 0){
            printf("setsockopt failed %s\n", strerror(errno));
            close(multicast_socket);
            close(open_socket);
            close(client_socket);
            continue;
        }

        break;
    }

    //the next of the last is NULL -> all failed
    if (addr == NULL){
        printf("Failed to set up the sockets\n");
        freeaddrinfo(res);
        exit(EXIT_FAILURE);
    }

    GAME_STATE game_state = IDLE;
    char grid[9];
    memset(grid, '-', 9);
    fd_set descriptors_set;
    FD_ZERO(&descriptors_set);
    FD_SET(STDIN_FILENO, &descriptors_set);
    FD_SET(open_socket, &descriptors_set);
    FD_SET(multicast_socket, &descriptors_set);
    FD_SET(client_socket, &descriptors_set);
    int max_fd = client_socket;
    int current_turn = 0;
    struct sockaddr_storage opponent;
    memset(&opponent, 0, sizeof(struct sockaddr_storage));
    socklen_t opponent_sock_len;
    while(running){
        if(select(max_fd + 1, &descriptors_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            exit(EXIT_FAILURE);
        }

        //check activiy in stdin
        if (FD_ISSET(STDIN_FILENO, &descriptors_set)){
            int bytes_in_stdin = 0;
            char *command_buffer = NULL;
            //check the number of bytes in stdin
            ioctl(STDIN_FILENO, FIONREAD, &bytes_in_stdin);
            bytes_in_stdin++;//make room for the \0
            if (bytes_in_stdin){
                command_buffer = (char *)malloc(sizeof(char) * bytes_in_stdin);
                if (NULL == fgets(command_buffer, bytes_in_stdin, stdin)){
                    printf("[ERROR] fgets failed\n");
                }
                command_buffer[bytes_in_stdin - 2] = '\0';
                int params[] = {0, 0};
                int command = parse_command(command_buffer, params);
                switch(command){
                    case SEARCH_COMMAND_CODE:
                        if (game_state == IDLE){
                            list_clear(players);
                            send_WHOIS(multicast_socket, (struct sockaddr*)&multicast_group, sizeof(multicast_group));
                        }else{
                            printf("ALREADY PLAYING\n");
                        }
                        break;
                    case GAMES_COMMAND_CODE:
                        if (game_state == IDLE){
                            list_print(players);
                        }else{
                            printf("ALREADY PLAYING\n");
                        }
                        break;
                    case JOIN_COMMAND_CODE:
                        if (params[0] < players->count){
                            node_t *player = list_get_node_by_index(params[0], players);
                            send_HELLO(client_socket, (struct sockaddr*)player->addr, sizeof(struct sockaddr));
                        }else{
                            printf("INVALID GAME NUMBER\n");
                        }
                        break;
                    case GRID_COMMAND_CODE:
                        if (game_state != IDLE){
                            print_grid(grid);
                        }else{
                            printf("NOT PLAYING\n");
                        }
                        break;
                    case PLACE_COMMAND_CODE:
                            if (game_state == SERVER && current_turn == 0){
                                if (params[0] >= 0 && params[0] < 3 && params[1] >= 0 && params[1] < 3 &&
                                    grid[params[0] * 3 + params[1]] == '-'){
                                    grid[params[0] * 3 + params[1]] = 'o';
                                    if (test_won(grid, 'o')){
                                        send_WINNER(open_socket, (struct sockaddr*)&opponent, opponent_sock_len, 'o');
                                    }else{
                                        send_GRID(open_socket, (struct sockaddr*)&opponent, opponent_sock_len, grid);
                                        current_turn = (current_turn + 1) % 2;
                                    }
                                }else{
                                    printf("[ERROR] Invalid position\n");
                                }
                            }else if(game_state == CLIENT && current_turn == 1){
                                if (grid[params[0] * 3 + params[1]] == '-'){
                                    send_POSITION(client_socket, (struct sockaddr*)&opponent, opponent_sock_len, params[0], params[1]);
                                }else{
                                    printf("[ERROR] Invalid position\n");
                                }
                            }else{
                                printf("NOT PLAYING\n");
                            }
                            break;
                    case QUIT_COMMAND_CODE:
                        if (game_state == IDLE){
                            running = 0;
                            list_delete(players);
                            freeaddrinfo(res);
                            free(players);
                        }else{
                            if(game_state == SERVER){
                                send_QUIT(open_socket, (struct sockaddr *)&opponent, opponent_sock_len);
                                perror("asd");
                                printf("YOU HAVE LEFT THE GAME\n");
                            }else if (game_state == CLIENT){
                                printf("YOU HAVE LEFT THE GAME\n");
                                send_QUIT(client_socket, (struct sockaddr *)&opponent, opponent_sock_len);
                                perror("zxc");
                            }else{

                            }
                            game_state = IDLE;
                        }
                        break;
                    default:
                        printf("[ERROR] Unknow command\n");
                        break;
                }
            }else{
                printf("[ERROR] Your STDIN is broken, fix it!\n");
            }

            if (command_buffer != NULL){
                free(command_buffer);
                command_buffer = NULL;
            }
        }

        //multicast activity
        if(FD_ISSET(multicast_socket, &descriptors_set)){
            struct sockaddr_storage *sender_address = (struct sockaddr_storage*)malloc(sizeof(struct sockaddr_storage));
            socklen_t sender_address_size = sizeof(struct sockaddr_storage);
            memset(sender_address, 0, sizeof(struct sockaddr_storage));
            //printf("MULTICAST\n");
            uchar recvbuffer[1024];
            memset(recvbuffer, 0, 1024);
            if (recvfrom(multicast_socket, &recvbuffer, 1024, 0, (struct sockaddr*)sender_address, &sender_address_size) < 0){
                perror("Recvfrom multicast");
            }

            if (game_state == IDLE){
                if(*(uint8_t*)recvbuffer == WHOIS){
                    send_HOSTINFO(multicast_socket, (struct sockaddr*)&multicast_group, sizeof(multicast_group), ((struct sockaddr_in *)addr->ai_addr)->sin_port);
                    free(sender_address);
                }else if(*(uint8_t*)recvbuffer == HOSTINFO){
                    ((struct sockaddr_in *)sender_address)->sin_port = *(uint16_t*)((uint8_t*)recvbuffer + 1);
                    node_t *new_player = list_create_node(sender_address);
                    list_add_last(new_player, players);
                }
            }else{
                free(sender_address);
            }
        }

        //activity in the server socket
        if (FD_ISSET(open_socket, &descriptors_set)){
            struct sockaddr_storage sender_address;
            memset(&sender_address, 0, sizeof(struct sockaddr_storage));
            socklen_t sender_address_size = sizeof(struct sockaddr_storage);
            uchar recvbuffer[1024];
            memset(recvbuffer, 0, 1024);
            //HAY QUE COMPROBAR EL REMITENTE!!!!
            if (recvfrom(open_socket, &recvbuffer, 1024, 0, (struct sockaddr*)&sender_address, &sender_address_size) < 0){
                perror("Recvfrom server socket");
            }

            if(game_state == IDLE){
                if(*(uint8_t*)recvbuffer == HELLO){
                    opponent = sender_address;
                    opponent_sock_len = sender_address_size;
                    memset(grid, '-', 9);
                    if (0 > send_HREPLY(open_socket, (struct sockaddr*)&sender_address, sender_address_size)){
                        perror("HREPLY");
                    }
                    game_state = SERVER;
                    current_turn = 0;
                }
                //when the game is in IDLE status you can only receive HELLO msgs, other types are ignored.
            }else if(!memcmp(&opponent, &sender_address, opponent_sock_len)){
                if(game_state == SERVER){
                    if(*(uint8_t*)recvbuffer == POSITION){
                        if (current_turn == 1){
                            int x = *((uint8_t*)recvbuffer + 1);
                            int y = *((uint8_t*)recvbuffer + 2);
                            if (x >= 0 && x < 3 && y >= 0 && y < 3 && grid[*((uint8_t*)recvbuffer + 1) * 3 + *((uint8_t*)recvbuffer + 2)] == '-'){
                                grid[*((uint8_t*)recvbuffer + 1) * 3 + *((uint8_t*)recvbuffer + 2)] = 'x';
                                if(test_won(grid, 'x')){
                                    send_WINNER(open_socket, (struct sockaddr*)&opponent, opponent_sock_len, 'x');
                                    printf("YOU WIN\n");
                                    game_state = IDLE;
                                }else{
                                    current_turn = (current_turn + 1) % 2;
                                    send_GRID(open_socket, (struct sockaddr*)&opponent, opponent_sock_len, grid);
                                }
                            }else{
                                send_ERROR(open_socket, (struct sockaddr*)&opponent, opponent_sock_len, 2);
                            }
                            //check winner
                        }else if(*(uint8_t*)recvbuffer == QUIT){
                            printf("REMOTE PLAYER LEFT\n");
                            game_state = IDLE;
                            memset(&opponent, 0, opponent_sock_len);
                        }else{
                            //error
                        }

                    }else if(*(uint8_t*)recvbuffer == QUIT){
                        game_state = IDLE;
                    }else{
                        send_ERROR(open_socket, (struct sockaddr*)&sender_address, sender_address_size, 1);
                    }
                }
            }else{
                send_ERROR(open_socket, (struct sockaddr*)&sender_address, sender_address_size, 1);
            }
        }

        //activiy in the client socket
        if(FD_ISSET(client_socket, &descriptors_set)){
            struct sockaddr_storage sender_address;
            socklen_t sender_address_size = sizeof(struct sockaddr_storage);
            uchar recvbuffer[1024];
            memset(recvbuffer, 0, 1024);

            if (recvfrom(client_socket, &recvbuffer, 1024, 0, (struct sockaddr*)&sender_address, &sender_address_size) < 0){
                perror("Recvfrom client socket");
            }
            if (game_state == IDLE){
                if(*(uint8_t*)recvbuffer == HREPLY){
                    opponent = sender_address;
                    opponent_sock_len = sender_address_size;
                    game_state = CLIENT;
                    current_turn = 0;
                }
            }else if(!memcmp(&opponent, &sender_address, opponent_sock_len)){
                if(*(uint8_t*)recvbuffer == WINNER){
                    printf("[WINNER] Player %c wins\n", *(char*)recvbuffer + 1);
                }else if(*(uint8_t*)recvbuffer == GRID){
                    memcpy(grid, recvbuffer + 1, 9);
                    print_grid(grid);
                    current_turn = (current_turn + 1) % 2;
                }else if(*(uint8_t*)recvbuffer == QUIT){
                    printf("REMOTE PLAYER LEFT\n");
                    game_state = IDLE;
                    memset(&opponent, 0, opponent_sock_len);
                }else if(*(uint8_t*)recvbuffer == ERROR){

                }
            }else{
                //the client doesn't send any errors
            }
        }

        //reset the set
        FD_ZERO(&descriptors_set);
        FD_SET(STDIN_FILENO, &descriptors_set);
        FD_SET(open_socket, &descriptors_set);
        FD_SET(multicast_socket, &descriptors_set);
        FD_SET(client_socket, &descriptors_set);
        max_fd = client_socket;
    }

    close(open_socket);
    close(multicast_socket);
    return 0;
}

void help(char *program){
    printf("%s -m <multicast address> -p <port> -o <open port>\n", program);
    return;
}

int is_number(char *str, int base, int *number)
{
    if (str != NULL){
        char *endptr;
        *number = strtol(str, &endptr, base);
        int return_value = (*str != '\0' && *endptr == '\0');
        return return_value;
    }
    return 0;
}

int parse_command(char *command, int params[2])
{
    if (!strcmp(command, SEARCH_COMMAND_STR)){
        return SEARCH_COMMAND_CODE;
    }else if(!strcmp(command, GAMES_COMMAND_STR)){
        return GAMES_COMMAND_CODE;
    }else if(!strncmp(command, JOIN_COMMAND_STR, 5)){
        params[0] = atoi((command + 6));
        params[1] = -1;
        return JOIN_COMMAND_CODE;
    }else if(!strcmp(command, GRID_COMMAND_STR)){
        return GRID_COMMAND_CODE;
    }else if(!strncmp(command, PLACE_COMMAND_STR, 6)){
        char x[] = {command[7], '\0'};
        char y[] = {command[9], '\0'};
        params[0] = atoi(x);
        params[1] = atoi(y);
        return PLACE_COMMAND_CODE;
    }else if(!strcmp(command, QUIT_COMMAND_STR)){
        return QUIT_COMMAND_CODE;
    }
    return UNKOWN_COMMAND_CODE;
}


void print_grid(char grid[9])
{

    printf("%c|%c|%c\n", grid[0], grid[3], grid[6]);
    printf("-----\n");
    printf("%c|%c|%c\n", grid[1], grid[4], grid[7]);
    printf("-----\n");
    printf("%c|%c|%c\n", grid[2], grid[5], grid[8]);
    printf("\n");
    return;
}

int test_won(char grid[9], char player)
{
    for (int i = 0; i < 3; i++){
        if (grid[i*3 + 0] == player && grid[i*3 +1] == player && grid[i*3 + 2] == player){
            return 1;
        }
    }
    for (int i = 0; i < 3; i++){
        if (grid[0 * 3 + i] == player && grid[1 * 3 + i] == player && grid[2 * 3 + i] == player){
            return 1;
        }
    }

    return (grid[0] == player && grid[4] == player &&  grid[8] == player) || (grid[2] == player && grid[4] == player &&  grid[6] == player);
}
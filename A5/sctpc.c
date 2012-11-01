/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              sctp.c               #
####################################################
*/

#define USE_CONNECT 0

#include "sctpc.h"
#include "protocol.h"
static volatile int running = 1;
int main(int argc, char* argv[])
{
    return client(argc, argv);
}

int client(int argc, char* argv[])
{
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);
    int sctp_sock = -1, gai_stat = 0;       // socket
    struct sockaddr_storage server_addr;        // server address structure
    //struct sockaddr_storage peer_addr;      // peer address structure
    // Lengths for address structures
    socklen_t server_addr_len = 0;
    struct sctp_event_subscribe sctp_events;    // sctp events
    struct sctp_sndrcvinfo infodata;        // sctp sender info
    struct addrinfo hints, *ai = NULL, *iter = NULL;  // for getaddrinfo

    // invalid amount of parameters
    if(argc != 3){
        return testclient_input_error((argc == 1) ? argv[0] : NULL);
    }

    // Use stroull for larger numbers, atoi() can be used for short ints
    unsigned int port = strtoull(argv[2], NULL, 10);
    printf("connecting to: %s %u\n", argv[1], port);
    memset(&server_addr, 0, sizeof(struct sockaddr_storage));
    // Get the server address structure with getaddrinfo()
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;            // To allow IPv4 or IPv6
    hints.ai_protocol = IPPROTO_SCTP;       // SCTP protocol supported
    hints.ai_socktype = SOCK_SEQPACKET;

    // Get (and set) server info
    if((gai_stat = getaddrinfo(argv[1], argv[2], &hints, &ai)) < 0) {
        fprintf(stderr, "Cannot resolve: ");
        print_addr_type(argv[1]);
        fprintf(stderr, "Error: %s\n", gai_strerror(gai_stat));
        if(ai){
            freeaddrinfo(ai);
        }
        return -1;
    }

    // Use the first of the returned addresses
    for(iter = ai; iter != NULL; iter = iter->ai_next) {
        // To allow the usage of both use IPv6 type for socket, create socket for SCTP
        if((sctp_sock = socket(iter->ai_family, iter->ai_socktype, iter->ai_protocol)) < 0) {
            perror("socket()");
            if(ai){
                freeaddrinfo(ai);
            }
            return -1;
        }
        memcpy(&server_addr, iter->ai_addr, iter->ai_addrlen);
        server_addr_len = iter->ai_addrlen;
        break;
    }

    freeaddrinfo(ai); // No need for this anymore
    // Allow to receive events from SCTP stack
    memset(&sctp_events, 0, sizeof(sctp_events));
    set_sctp_events(&sctp_events);
    if(setsockopt(sctp_sock, IPPROTO_SCTP, SCTP_EVENTS, &sctp_events, sizeof(sctp_events)) < 0){
        perror("setsockopt");
    }



    char mbuffer[513];
    memset(&mbuffer, 0, 513);
    int mlen = 513;
    int mflags = 0;

    // For select()
    fd_set incoming;
    char *command_buffer = NULL;
    int joined = 0;
    int in_game = 0;
    int ready = 0;
    int player_id = 0;
    int area_rows = 0;
    int area_columns = 0;
    char *area = NULL;
    while(running){
        FD_ZERO(&incoming);
        FD_SET(sctp_sock, &incoming);
        FD_SET(STDIN_FILENO, &incoming);
        switch(select(sctp_sock + 1,  &incoming, NULL, NULL, NULL)){
            case -1:
                perror("Select failed, terminating");
                running = 0;
                break;
            case 0:
                break;
            default:
            {
                if(FD_ISSET(sctp_sock, &incoming)){
                    int bytes = sctp_recvmsg(sctp_sock, mbuffer, mlen, (SA*)&server_addr, &server_addr_len, &infodata, &mflags);

                    if(mflags & MSG_NOTIFICATION){
                        if (bytes < 8){
                            continue;
                        }
                        switch(*(uint16_t*)mbuffer){
                            case SCTP_SHUTDOWN_EVENT:
                                running = 0;
                            break;
                            case SCTP_PEER_ADDR_CHANGE:
                            switch(((struct sctp_paddr_change *)mbuffer)->spc_state){
                                //if the primary addr has changed, update the list
                                //we dont want to end up with a dead addr in the database
                                case SCTP_ADDR_MADE_PRIM:
                                    printf("ADDR CHANGE\n");
                                    break;
                            }
                            break;
                        }
                    }else{
                        //ignore wrong PPID
                        if (ntohl(infodata.sinfo_ppid) != PROTOCOL_PAYLOAD_IDENTIFER){
                            continue;
                        }

                        if(ntohs(*(int16_t*)mbuffer) == OK_MSG){
                            joined = 1;
                            player_id = *((int8_t*)((int16_t*)mbuffer + 1) + 1);
                            printf("[PLAYER ID]:%d\n", player_id);
                        }else if(ntohs(*(int16_t*)mbuffer) == START_MSG){
                            area_columns = *(int8_t*)((int16_t*)mbuffer + 1);
                            area_rows    = *((int8_t*)((int16_t*)mbuffer + 1) + 1);
                            in_game      = 1;
                            system("clear");
                        }else if(ntohs(*(int16_t*)mbuffer) == ERROR_MSG){
                            printf("[ERROR] %s\n", (char*)((int8_t*)((int16_t*)mbuffer + 1) + 1));
                        }else if(ntohs(*(int16_t*)mbuffer) == TURN_MSG){
                            printf("[YOUR TURN]\n");
                        }else if(ntohs(*(int16_t*)mbuffer) == WINNER_MSG){
                            int winner = *(int8_t*)((int16_t*)mbuffer + 1);
                            if (winner == player_id){
                                printf("[YOU WON]\n");
                            }else if(winner > 0){
                                printf("[PLAYER %d WON]\n", winner);
                            }else{
                                printf("[DRAW]\n");
                            }
                        }else if(ntohs(*(int16_t*)mbuffer) == AREA_MSG){
                            if (area != NULL){
                                free(area);
                                area = NULL;
                            }
                            area = unpack_area((char*)((int16_t*)mbuffer + 1), area_rows, area_columns);
                            system("clear");
                            print_grid(area, area_rows, area_columns);
                        }
                    }
                }

                if(FD_ISSET(STDIN_FILENO, &incoming)){
                    int bytes_in_stdin = 0;
                    //check the number of bytes in stdin
                    ioctl(STDIN_FILENO, FIONREAD, &bytes_in_stdin);
                    if (bytes_in_stdin){
                        bytes_in_stdin++;//make room for the \0
                        command_buffer = (char *)malloc(sizeof(char) * bytes_in_stdin);
                        if (NULL == fgets(command_buffer, bytes_in_stdin, stdin)){
                            printf("[ERROR] fgets failed\n");
                        }
                        command_buffer[bytes_in_stdin - 2] = '\0';
                        int params[] = {0};
                        int command = parse_command(command_buffer, params);
                        switch(command){
                            case JOIN_COMMAND_CODE:
                                if(!joined){
                                    send_join (sctp_sock, (SA*)&server_addr, server_addr_len);
                                }else{
                                    printf("Already connected\n");
                                }
                                break;
                            case READY_COMMAND_CODE:
                                if(!ready && joined){
                                    send_ready(sctp_sock, (SA*)&server_addr, server_addr_len);
                                }
                                break;
                            case AREA_COMMAND_CODE:
                                if (in_game){
                                    print_grid(area, area_rows, area_columns);
                                }else{
                                    printf("Not playing\n");
                                }
                                break;
                            case PLACE_COMMAND_CODE:
                                if(in_game){
                                    send_column(sctp_sock, params[0], (SA*)&server_addr, server_addr_len);
                                }else{
                                    printf("Not playing\n");
                                }
                                break;
                            case QUIT_COMMAND_CODE:
                                running = 0;
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
            }
        }
    }

    if (command_buffer != NULL){
        free(command_buffer);
        command_buffer = NULL;
    }
    drop_connection(sctp_sock, (SA*)&server_addr, server_addr_len);
    close(sctp_sock);
    return 0;
}

int testclient_input_error(char* prgrm)
{
    printf("Not enough parameters");
    if(prgrm){
        printf(", use: %s <server ipv4/ipv6> <serverport>", prgrm);
    }
    printf("\n");
    return -1;
}


void print_addr_type(char* address) {
    // Check that given address is either IPv4 or IPv6 type
    switch(check_addr_type(address, strlen(address))){
        case 0:
            #ifdef __DEBUG_EN
                printf("invalid address\n");
            #endif
            break;
        case 1:
            #ifdef __DEBUG_EN
                printf("interface not provided along IPv6 link local address\n");
            #endif
            break;
        case 2:
            #ifdef __DEBUG_EN
                printf("Invalid IPv6 address (invalid length)\n");
            #endif
            break;
        case 4:
            #ifdef __DEBUG_EN
                printf("IPv4 address\n");
            #endif
            break;
        case 6:
            #ifdef __DEBUG_EN
                printf("IPv6 address\n");
            #endif
            break;
        case 10:
            #ifdef __DEBUG_EN
                printf("IPv6 loopback address\n");
            #endif
            break;
    }
}

int check_addr_type(char* addr, int len)
{
    if(!addr){
        return -1;
    }
    // Assume that ':' exist multiple times and is IPv6 addr
    if(strchr(addr, ':')){
        if(len < 8){
            return 10;  // Loopback IPv6 (0000::1 / 0000:: / ::1)
        } else if((len >= 14) && (len <= INET6_ADDRSTRLEN)){ // Should be at least IPv4-mapped ::ffff:0.0.0.0
            char ipv6prefix[5];
            memset(&ipv6prefix, 0, 5);
            // Copy 4 first chars and check for known prefixes (link local needed atm)
            memcpy(ipv6prefix, addr, 4);
            ipv6prefix[4] = '\0';
             // Link local IPv6 address
            if(strcmp(ipv6prefix, "fe80") == 0){
                if(strchr(addr, '%')) {
                    return 6; // Interface defined for link local IPv6 addr
                }else{
                    return 1;
                }
            }else{
                return 6; // Otherwise assume that it is an IPv6 address, getaddrinfo reports validity
            }
        }else{
            return 2; // The length for IPv6 address is not valid
        }
    }else{
        if(len < 7){
            return 0; // should be at least 0.0.0.0
        }
        int pos = 0;
        // at least 3 '.'
        for(int i = 0; i < 3; i++){
            char* tmp = strchr(&addr[pos], '.');
            if(tmp){
                pos = len - (strlen(tmp)) + 1;
            }else{
                return 0; // No more '.' found
            }
        }
        return 4;
    }
}

void sighandler(int sig)
{
    printf("\nQuiting...\n");
    running = 0;
}
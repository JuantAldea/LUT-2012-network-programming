/*
* CT30A5001 Network Programming
* sctps.c, STCP server and client example
*
* Contains simple STCP server that the client connects to. Echoes the sent data
* back to client and shows all available addresses for it. Actively listens for
* notificatioins from SCTP kernel.
*
* Author:
*   Jussi Laakkonen
*   1234567
*   jussi.laakkonen@lut.fi
*/


#include "sctps.h"

#define __BINDX_SEPARATELY

// Main loop global - Ctrl-C changes this to 0
static volatile int running = 1;

int main(int argc, char* argv[])
{
#ifdef __RUN_TESTS
    return run_test_server(argc, argv);
#else
    // Call your server code here
    return 0;
#endif
}

int run_test_server(int argc, char* argv[])
{
    int sctp_otm_sock = -1, port_otm = -1;      // socket and port for one to many sctp
    int count = 0;
    char mbuffer[MBUFFERLEN];           // static message buffer
    struct sockaddr* addresses = NULL;
    struct sctp_event_subscribe sctp_events;    // sctp events
    struct sctp_sndrcvinfo infodata;        // information about sender
    struct sctp_initmsg sctp_init;          // sctp socket option initialization
    struct sockaddr *packedlist = NULL;     // packed list of addresses

    // Add listening for 2 interruption signals
    signal(SIGTERM, sighandler);
    signal(SIGINT, sighandler);

    // not enough or too many parameters
    if(argc > 3 || argc < 2){
        return testserver_input_error((argc == 1) ? argv[0] : NULL);
    }

    // Port is not valid
    if((port_otm = atoi(argv[1])) <= 0x0400 || port_otm > 0xffff){
        return testserver_input_error(NULL);
    }

    printf("starting server on port:\t%d\n", port_otm);

    // Socket creation
    if((sctp_otm_sock = socket(PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP)) == -1){
        return error_situation();
    }

    if(argc == 3) {
        if(strcasecmp(argv[2], "ifaddrs") == 0){
            addresses = get_own_addresses_gia(&count, port_otm, NOLOOPBACK, ONLYRUNNING);
        }else if(strcasecmp(argv[2], "addrinfo") == 0){
            addresses = get_own_addresses_gai(&count, argv[1]);
        }else if(strcasecmp(argv[2], "combined") == 0){
            addresses = get_own_addresses_combined(&count, argv[1], NOLOOPBACK, ONLYRUNNING);
        }else {
            printf("Invalid parameter for address search function selection.\n \
                \tifaddrs = getifaddrs()\n\taddrinfo = getaddrinfo()\n");
            exit(1);
        }
    }else{
        addresses = get_own_addresses_gia(&count, port_otm, NOLOOPBACK, ONLYRUNNING);
    }

    print_packed_addresses(count, (struct sockaddr_storage*)addresses);

    if(!addresses){
        printf("No addresses to bind\n");
    }
#ifdef __BINDX_SEPARATELY
    else {
        int i = 0, storlen = 0, slen = 0;
        // Start from the beginning
        struct sockaddr* temp = addresses;
        while (i < count) {
            // We need to know the size
            slen = (temp->sa_family == AF_INET ? sizeof(struct sockaddr_in) :
                    temp->sa_family == AF_INET6 ? sizeof(struct sockaddr_in6) : 0);
            // Attempt to bind one address we're pointing at the moment
            if(sctp_bindx(sctp_otm_sock, temp, 1, SCTP_BINDX_ADD_ADDR) != 0) {
                perror("Bindx failed");
                break;
            } else {
                printf("Added address");
                print_packed_addresses(1, (struct sockaddr_storage*)temp);
            }

            i++;
            storlen += slen; // Add the size of current struct
            temp = (struct sockaddr*)((char*)temp + slen); // Move pointer to next struct
        }
    }
#else
    else if (sctp_bindx(sctp_otm_sock, addresses, count, SCTP_BINDX_ADD_ADDR) != 0){
        perror("Bindx failed");
    }
#endif

    if(addresses){
        free(addresses); // No need for this anymore
    }

    // Set to receive events
    memset(&sctp_events, 0, sizeof(sctp_events));
    set_sctp_events(&sctp_events);
    if(setsockopt(sctp_otm_sock, IPPROTO_SCTP, SCTP_EVENTS,  &sctp_events, sizeof(sctp_events)) != 0){
        perror("Setsockopt failed");
    }

    // Set stream count and attempt counter
    memset(&sctp_init, 0, sizeof(sctp_init));
    sctp_init.sinit_num_ostreams = 3;   // out streams
    sctp_init.sinit_max_instreams = 3;  // in streams
    sctp_init.sinit_max_attempts = 2;   // maximum number for connection attempts

    if(setsockopt(sctp_otm_sock, IPPROTO_SCTP, SCTP_INITMSG,  &sctp_init, sizeof(sctp_init))){
        perror("Setsockopt failed");
    }

    // Start listening
    if(listen(sctp_otm_sock, 10) != 0){
        perror("Listen failed");
    }

    // For select()
    fd_set incoming;
    struct timeval to;
    memset(&to, 0, sizeof(to));
    to.tv_usec = 100 * 5000;
    to.tv_sec = 0;
    time_t time_since_scheduled_game = 0;
    // For sender address check
    struct sockaddr_storage sender = {0};

    linked_list_t *player_list = (linked_list_t *) malloc(sizeof(linked_list_t));
    list_init(player_list);
    int start_game = 0;
    int in_game = 0;
    int schedule_start = 0;
    char *area = NULL;
    int columns, rows;

    while(running){
        if (schedule_start && (time(NULL) - time_since_scheduled_game) > 3){
            schedule_start = 0;
            start_game = 1;
        }

        //all the things that should be done before entering the real game
        // a game could be started by schedule (full of players) or by ready check
        if (start_game){
            printf("START GAME\n");
            list_set_movement_order(player_list);
            list_print(player_list);
            list_sort_by_turn(player_list);
            list_print(player_list);
            if (player_list->count == 2){
                area = malloc(sizeof(char) * 7 * 6);
                columns = 7;
                rows = 6;
            }else if(player_list->count > 2){
                area = malloc(sizeof(char) * 9 * 7);
                columns = 9;
                rows = 7;
            }
            broadcast_start(sctp_otm_sock, columns, rows, player_list);
            start_game = 0;
            in_game = 1;
        }

        FD_ZERO(&incoming);
        FD_SET(sctp_otm_sock, &incoming);
        switch(select(sctp_otm_sock + 1,  &incoming, NULL, NULL,  &to)){
            case -1:
                perror("Select failed, terminating");
                running = 0;
                break;
            case 0:
                break;
            default:
            {
                memset(&sender, 0, sizeof(sender));
                memset(&infodata, 0, sizeof(infodata));
                memset(&mbuffer, 0, MBUFFERLEN);

                unsigned int salen = sizeof(sender);
                int mlen = MBUFFERLEN;
                int mflags = 0;

                // Receive the message
                int bytes = sctp_recvmsg(sctp_otm_sock, mbuffer, mlen, (struct sockaddr*)&sender, &salen, &infodata, &mflags);
                // Received a SCTP notification
                if(mflags & MSG_NOTIFICATION){
                    printf("A notification from association %d\n",  infodata.sinfo_assoc_id);
                    if (bytes < 8){
                        continue;
                    }
                    switch(*(uint16_t*)mbuffer){
                        case SCTP_SHUTDOWN_EVENT:
                        {
                            node_t *player = list_get_node_by_session_id(((struct sctp_shutdown_event*)mbuffer)->sse_assoc_id, player_list);
                            if(player != NULL){
                                printf("SHUTDOWN\n");
                                list_remove_node(player, player_list);
                                list_print(player_list);
                            }
                        }
                        break;
                        case SCTP_PEER_ADDR_CHANGE:
                            switch(((struct sctp_paddr_change *)mbuffer)->spc_state){
                                //if the primary addr has changed, update the list
                                //we dont want to end up with a dead addr in the database
                                case SCTP_ADDR_MADE_PRIM:
                                    memcpy(
                                        &list_get_node_by_session_id(((struct sctp_paddr_change *)mbuffer)->spc_assoc_id, player_list)->sa,
                                        &((struct sctp_paddr_change *)mbuffer)->spc_aaddr,
                                        sizeof(struct sockaddr_storage)
                                    );
                                    break;
                            }
                    }
                    check_sctp_event(mbuffer, bytes);
                }else{// Something else from a client
                    int16_t message_code = ntohs(*(int16_t*)mbuffer);
                    if(message_code == JOIN_MSG){
                        if (player_list->count < 4){
                            printf("JOIN\n");
                            int id = list_get_first_free_player_id(player_list);
                            assert(id >= 0);
                            list_add_last(list_create_node(id, infodata.sinfo_assoc_id, sender), player_list);
                            list_print(player_list);
                            send_ok(sctp_otm_sock, player_list->count, id, (struct sockaddr*)&sender, salen);
                            if (player_list->count == 4){
                                schedule_start = 1;
                                time_since_scheduled_game = time(NULL);
                            }
                        }else{
                            printf("SERVER FULL\n");
                            char error_msg[] = "Game is full\0";
                            int a = send_error(sctp_otm_sock, ERROR_GAME_FULL, error_msg, (struct sockaddr*)&sender, salen);
                            drop_connection (sctp_otm_sock, (struct sockaddr*)&sender, salen);
                        }
                    }else if(message_code == READY_MSG){
                        node_t *player = list_get_node_by_session_id(infodata.sinfo_assoc_id, player_list);
                        if (player != NULL){
                            printf("READY\n");
                            player->ready = 1;
                            list_print(player_list);
                        }
                        //TODO enable ready check start
                        //start_game = list_ready_check(player_list) && (player_list->count > 1);
                    }else if(message_code == COLUMN_MSG){

                    }


                    printf("\n\t%d bytes from association %u\n", bytes, infodata.sinfo_assoc_id );
                    // Get all addresses of the peer
                    int addr_count = sctp_getpaddrs(sctp_otm_sock, infodata.sinfo_assoc_id, &packedlist);

                    if(addr_count < 0){
                        perror("sctp_getpaddrs failed");
                    }

                    // Send the message back to client
                    //bytes = sctp_sendmsg(sctp_otm_sock, &mbuffer, bytes, packedlist, salen, infodata.sinfo_ppid, infodata.sinfo_flags, infodata.sinfo_stream++, 0, 0);
                    // Free the addresses of the peer
                    sctp_freepaddrs(packedlist);
                    packedlist = NULL;
                }
                break;
            }
        }
        usleep(1000);
    }
    list_delete(player_list);
    free(player_list);
    free(area);
    close(sctp_otm_sock);
    return 0;
}


void sighandler(int sig)
{
    printf("\nCaught an interrupt (%s),  doing shutdown in 1 second\n",  (sig == SIGTERM) ? "SIGTERM" : "SIGINT" );
    running = 0;
}

int testserver_input_error(char* prgrm)
{
    printf("Not enough parameters or invalid parameters");
    if(prgrm){
        printf(",  use: %s <server_one_to_many_port>",  prgrm);
    }
    printf("\n");
    return -1;
}

int broadcast_start(int sctp_socket, int8_t columns, int8_t rows, linked_list_t *list)
{
    int error = 0;
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        int salen = 0;
        if(i->sa.ss_family == AF_INET){
            salen = sizeof(struct sockaddr_in);
        }else if (i->sa.ss_family == AF_INET6){
            salen = sizeof(struct sockaddr_in6);
        }
        int bytes = send_start(sctp_socket, columns, rows, (SA*)&i->sa, salen);
        if (bytes < 0){
            error = 1;
            perror("Error broadcasting start:");
        }
    }
    return error;
}
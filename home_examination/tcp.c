#include "tcp.h"

int prepare_server_TCP (char *port, short family)
{
    struct addrinfo hints;
    //fill all the fields with zero, just to be sure. Actually it crashed without this.
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;//don't care ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; //automatic self ip
    hints.ai_protocol = IPPROTO_TCP;
    int error = 0;
    struct addrinfo *res = NULL;
    if ((error = getaddrinfo(NULL, port, &hints, &res)) < 0){
        printf("Getaddrinfo error: %s\n", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    int socket_descriptor = -1;
    struct addrinfo *ptr = NULL;
    //loop looking for a addr that works
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

        if (bind(socket_descriptor, ptr->ai_addr, ptr->ai_addrlen) < 0) {
            printf("bind failed %s\n", strerror(errno));
            close(socket_descriptor);
            continue;
        }

        if (listen(socket_descriptor, BACKLOG) < 0){
            printf("Error listening %s\n", strerror(errno));
            close(socket_descriptor);
            continue;
        }
        break;
    }

    freeaddrinfo(res);
    //the next of the last, NULL -> all failed
    if (ptr == NULL){
        perror("Bind failed\n");
        exit (EXIT_FAILURE);
    }

    return socket_descriptor;
}

int prepare_connection_TCP(char *address, char *port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
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
        printf("Error no address found\n");
        return -1;
    }
    return socket_control;
}

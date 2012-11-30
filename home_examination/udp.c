#include "udp.h"

int prepare_server_UDP (char *port)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    int error = 0;
    struct addrinfo *res = NULL;
    if ((error = getaddrinfo(NULL, port, &hints, &res)) < 0){
        printf("Getaddrinfo error: %s\n", gai_strerror(error));
        exit(EXIT_FAILURE);
    }

    int socket_descriptor = -1;
    struct addrinfo *ptr = NULL;
    //loop looking for an addr that works
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
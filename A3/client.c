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

int client(char *name, char *address, char *port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //only IPv6 allowed
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_UDP;

	int error = 0;
	struct addrinfo *res = NULL;
	if ((error = getaddrinfo(address, port, &hints, &res)) < 0){
		perror(gai_strerror(error));
		freeaddrinfo(res);
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
        break;
    }




    if (ptr == NULL){
	    freeaddrinfo(res);
	    printf("error\n");
    	return -1;
    }

    char dgram[] = "ADDBroza\0";
    sendto(socket_descriptor, &dgram, 9, 0, ptr->ai_addr, ptr->ai_addrlen);
    freeaddrinfo(res);
	close(socket_descriptor);
	return EXIT_SUCCESS;
}

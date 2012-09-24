#include "client.h"

int client_parse_command(char *command)
{
	if(!strncmp(command, QUIT_COMMAND, strlen(QUIT_COMMAND))){
		return QUIT_COMMAND_CODE;
	}else{
		return UNKOWN_COMMAND_CODE;
	}
}

int client(char *name, char *address, char *port)
{
	int socket_descriptor = -1;
	if ((socket_descriptor = prepare_connection(address, port)) < 0){
		return EXIT_FAILURE;
	}
	send_login(socket_descriptor, name, name);
	fd_set master_set;
	fd_set ready_set;
	FD_ZERO(&master_set);
	FD_ZERO(&ready_set);
	int number_of_fds = socket_descriptor + 1;// fd 0 is stdin, so we have one fd more
	FD_SET(STDIN_FILENO, &master_set);
	FD_SET(socket_descriptor, &master_set);
	int running = 1;
	recv_buffer_t *buffer = (recv_buffer_t *)malloc(sizeof(recv_buffer_t));
	while(running){
		ready_set = master_set;
		//select descriptors with activity
		if(select(number_of_fds, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            return EXIT_FAILURE;
        }
        //search if the descriptor i has activity
		for (int i = 0; i < number_of_fds; i++){
			if (FD_ISSET(i, &ready_set)){
				if (i == STDIN_FILENO){
					char command_buffer[10];
					//don't flood me!
					scanf("%9s", command_buffer);
					fflush(stdin);
					int command = client_parse_command(command_buffer);
					switch(command){
						case QUIT_COMMAND_CODE:
							printf("Quitting...(if you are running valgrind you may see an invalid free() in the glibc 2.16 scope)\n");
							running = 0;
							break;
						default:
							break;
					}
				}else{
					// //char *buffer = NULL;
					// uint8_t type;
					// int full_message = 0;
					// int recv_bytes = recv_msg(i, buffer, &full_message);
					// if (recv_bytes == 0){//read of size 0->disconect
					// 	printf("Conection closed by remote host\n");
					// 	close(socket_descriptor);
					// 	return EXIT_SUCCESS;
					// }
					// else if (recv_bytes > 0){
					// 	if (type == ERROR_MSG){
					// 		printf("Error: %s\n", buffer->buffer);
					// 	}else{
					// 		printf("MSG: %s\n", buffer->buffer);
					// 	}
					// 	free(buffer);
					// }else{
					// 	printf("Error\n");
					// 	close(socket_descriptor);
					// 	return EXIT_FAILURE;
					// }
				}
			}
		}
	}
	char broza[] = "Me voy";
	send_disconnect(socket_descriptor, broza);
	close(socket_descriptor);
	return EXIT_SUCCESS;
}

int prepare_connection(char *address, char *port)
{
	struct addrinfo hints;
	//fill all the fields with zero, just to be sure. Actually it crashed without this.
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; //don't care ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //automatic self ip
	hints.ai_protocol = IPPROTO_TCP;

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

        int option_value = 1;
        if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value)) < 0){
        	printf("setsockopt failed %s\n", strerror(errno));
			close(socket_descriptor);
    	    continue;
		}

		if(connect(socket_descriptor, ptr->ai_addr, ptr->ai_addrlen) < 0){
			printf("connect failed %s\n", strerror(errno));
			close(socket_descriptor);
			continue;
		}
        break;
    }
    freeaddrinfo(res);
    //if no addr is found, we have a problem
    if (ptr == NULL){
    	return -1;
    }
    return socket_descriptor;
}
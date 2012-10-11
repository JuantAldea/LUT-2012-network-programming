/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 server.c                    #
###############################################
*/

#include "server.h"
#include <sys/ioctl.h>

int parse_command(char *command)
{
	if (!strcmp(command, LIST_COMMAND_STR)){
		return LIST_COMMAND_CODE;
	}else if(!strcmp(command, START_COMMAND_STR)){
		return START_COMMAND_CODE;
	}else if(!strcmp(command, STOP_COMMAND_STR)){
		return STOP_COMMAND_CODE;
	}else if(!strcmp(command, SHUTDOWN_COMMAND_STR)){
		return SHUTDOWN_COMMAND_CODE;
	}else{
		return UNKOWN_COMMAND_CODE;
	}
}

int server(char *port)
{
	linked_list_t *users = (linked_list_t*)malloc(sizeof(linked_list_t));
	list_init(users);

	int listening_socket = -1;
	if ((listening_socket = prepare_server(port)) < 0){
		exit(EXIT_FAILURE);
	}

	fd_set descriptors_set;
	FD_ZERO(&descriptors_set);
	FD_SET(STDIN_FILENO, &descriptors_set);
	FD_SET(listening_socket, &descriptors_set);

	int max_fd = listening_socket;
	int running = 1;//server running
	printf("[SERVER] Listening for connections on port %s (fd = %d)\n", port, listening_socket);
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
			if (bytes_in_stdin){
				//read (+ flush, all in one)
				command_buffer = (char *)malloc(sizeof(char) * bytes_in_stdin);
				if (NULL == fgets(command_buffer, bytes_in_stdin, stdin)){
					printf("[ERROR] fgets failed\n");
				}
			}
			if (bytes_in_stdin){
				int command = parse_command(command_buffer);
				switch(command){
					case SHUTDOWN_COMMAND_CODE:
						running = 0;
						break;
					case LIST_COMMAND_CODE:
						//list_print(users);
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
		//check activity in the listening socket
		if (FD_ISSET(listening_socket, &descriptors_set)){
			char recvbuffer[509];
			int size = 512;
			struct sockaddr_storage client_addr;
			socklen_t address_len = sizeof(client_addr);

			int asd = recvfrom(listening_socket, recvbuffer, size, 0, (struct sockaddr*)&client_addr, &address_len);
			printf("|%d|\n", asd);
			if(!strncmp(recvbuffer, "ADD", 3)){
				printf("añadiendo\n");
			}else if(!strncmp(recvbuffer, "GET", 3)){
				printf("obteniendo\n");
			}else{
			}

			char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
			if (getnameinfo((struct sockaddr*)&client_addr, sizeof(struct sockaddr), hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0){
				printf("host=%s, serv=%s\n", hbuf, sbuf);
			}
    		printf("%s\n", recvbuffer);
		}
		//reset the set
		FD_ZERO(&descriptors_set);
		//add all the descriptors to it
		FD_SET(STDIN_FILENO, &descriptors_set);
		FD_SET(listening_socket, &descriptors_set);
		//also set the max descriptor
		max_fd = listening_socket;
	}

	//at the end, disconnect everyone, clear all.
	close(listening_socket);
	return 0;
}

//returns the listening socket
int prepare_server (char *port)
{
	struct addrinfo hints;
	//fill all the fields with zero, just to be sure. Actually it crashed without this.
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
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

// never use fflush(stdin), the outcome is undefined
// fflush should be used only with output streams
void flush_stdin(void)
{
	int bytes_in_stdin = 0;
	ioctl(STDIN_FILENO, FIONREAD, &bytes_in_stdin);
	if (bytes_in_stdin){
		char *garbage = (char *)malloc(sizeof(char) * bytes_in_stdin);
		if (NULL == fgets(garbage, bytes_in_stdin, stdin)){
			printf("[ERROR] fgets failed\n");
		}
		free(garbage);
	}
}

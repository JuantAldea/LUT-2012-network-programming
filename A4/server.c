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

#include "server.h"
#include <sys/ioctl.h>
#include "common.h"
#include "server.h"

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
		return server(port);
	}else{
		printf("Wrong sintax\n");
		help(argv[0]);
	}
	return 0;
}


int server(char *port)
{
	linked_list_t *list_aphorisms = (linked_list_t*)malloc(sizeof(linked_list_t));
	list_init(list_aphorisms);

	int listening_socket = -1;
	uint16_t port_number = 0;
	if ((listening_socket = prepare_server(port, &port_number)) < 0){
		exit(EXIT_FAILURE);
	}
	printf("%d\n", port_number);
	fd_set descriptors_set;
	FD_ZERO(&descriptors_set);
	FD_SET(STDIN_FILENO, &descriptors_set);
	FD_SET(listening_socket, &descriptors_set);

	int max_fd = listening_socket;
	int running = 1;//server running

	GAME_STATE game_state = IDLE;
	while(running){
		if(select(max_fd + 1, &descriptors_set, NULL, NULL, NULL) < 0) {
			perror("Error in select");
			exit(EXIT_FAILURE);
		}
		//check activiy in stdin
		if (FD_ISSET(STDIN_FILENO, &descriptors_set)){
			cli();
		}
		//check activity in the listening socket
		if (FD_ISSET(listening_socket, &descriptors_set)){
			uchar recvbuffer[20];
			int size = 20;
			struct sockaddr_storage client_addr;
			socklen_t address_len = sizeof(client_addr);

			int error = recvfrom(listening_socket,
								recvbuffer,
								size,
								0,
								(struct sockaddr*)&client_addr,
								&address_len);
			if (error < 0){
				printf("[ERROR] Network error\n");
				return -1;
			}

			if(*(uint8_t*)recvbuffer == WHOIS){
				if (game_state == IDLE){
					send_HOSTINFO(listening_socket,
								  (struct sockaddr*)&client_addr,
								  address_len,
								  port_number);
				}
			}else if(*(uint8_t*)recvbuffer == HOSTINFO){

			}else if(*(uint8_t*)recvbuffer == HELLO){
				if(game_state == IDLE){
					//send_HREPLY();
				}else{
					//send_ERROR();
				}
			}else if(*(uint8_t*)recvbuffer == HREPLY){

			}else if(*(uint8_t*)recvbuffer == POSITION){

			}else if(*(uint8_t*)recvbuffer == GRID){

			}else if(*(uint8_t*)recvbuffer == WINNER){
				if(game_state == PLAYING){
					game_state = IDLE;
				}
			}else if(*(uint8_t*)recvbuffer == QUIT){
				if (game_state == PLAYING){
					game_state = IDLE;
				}
			}else if(*(uint8_t*)recvbuffer == ERROR){

			}else{
				printf("[ERROR] Unkown command\n");
				//char error_str[] = "Unkown command\0";
				//send_ERR(listening_socket, (struct sockaddr*)&client_addr, address_len, error_str);
			}

		}
		//reset the set
		FD_ZERO(&descriptors_set);
		//add all the descriptors to it
		FD_SET(STDIN_FILENO, &descriptors_set);
		FD_SET(listening_socket, &descriptors_set);
		//also set the max descriptor
		max_fd = listening_socket;
	}
	close(listening_socket);
	return 0;
}

//returns the listening socket
int prepare_server (char *port, uint16_t *port_number)
{
	struct addrinfo hints;
	//fill all the fields with zero, just to be sure. Actually it crashed without this.
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

    //the next of the last is NULL -> all failed
    if (ptr == NULL){
    	perror("Bind failed\n");
    	freeaddrinfo(res);
    	exit(EXIT_FAILURE);
    }

	//http://stackoverflow.com/a/2372149
	if (ptr->ai_family == AF_INET) {
		*port_number = ntohs(((struct sockaddr_in*)ptr->ai_addr)->sin_port);
	}else if(ptr->ai_family == AF_INET6){
		*port_number = ntohs(((struct sockaddr_in6*)ptr->ai_addr)->sin6_port);
	}

	freeaddrinfo(res);

	return socket_descriptor;
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

void cli()
{
	int bytes_in_stdin = 0;
	char *command_buffer = NULL;
	//check the number of bytes in stdin
	ioctl(STDIN_FILENO, FIONREAD, &bytes_in_stdin);
	if (bytes_in_stdin){
		command_buffer = (char *)malloc(sizeof(char) * bytes_in_stdin);
		if (NULL == fgets(command_buffer, bytes_in_stdin, stdin)){
			printf("[ERROR] fgets failed\n");
		}

		int command = parse_command(command_buffer);
		switch(command){
			case SEARCH_COMMAND_CODE:
				break;
			case GAMES_COMMAND_CODE:
				break;
			case JOIN_COMMAND_CODE:
				break;
			case GRID_COMMAND_CODE:
				break;
			case PLACE_COMMAND_CODE:
				break;
			case QUIT_COMMAND_CODE:
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

int parse_command(char *command)
{
    if (!strcmp(command, SEARCH_COMMAND_STR)){
        return SEARCH_COMMAND_CODE;
    }else if(!strcmp(command, GAMES_COMMAND_STR)){
        return GAMES_COMMAND_CODE;
    }else if(!strcmp(command, JOIN_COMMAND_STR)){
        return JOIN_COMMAND_CODE;
    }else if(!strcmp(command, GRID_COMMAND_STR)){
        return GRID_COMMAND_CODE;
    }else if(!strcmp(command, PLACE_COMMAND_STR)){
        return PLACE_COMMAND_CODE;
    }else if(!strcmp(command, QUIT_COMMAND_STR)){
        return QUIT_COMMAND_CODE;
    }
	return UNKOWN_COMMAND_CODE;
}



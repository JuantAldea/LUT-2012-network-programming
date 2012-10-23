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

	int port_number;
	while ((optc = getopt(argc, argv, "p:")) != -1) {
		switch (optc) {
			case 'p':
				if (is_number(optarg, 10, &port_number)){
					port = optarg;
					printf("Port number: %d\n", port_number);
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


	if (port != NULL){
			return server(port);
	}else{
		printf("Wrong sintax\n");
		help(argv[0]);
	}
	return 0;
}


int parse_command(char *command)
{
	if (!strcmp(command, LIST_COMMAND_STR)){
		return LIST_COMMAND_CODE;
	}else if(!strcmp(command, SHUTDOWN_COMMAND_STR)){
		return SHUTDOWN_COMMAND_CODE;
	}else{
		return UNKOWN_COMMAND_CODE;
	}
}

void load_database(FILE *file, linked_list_t *database)
{
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, file)) != -1) {
		char ip[NI_MAXHOST];
		char date[1024];
		char aphorism[509];
		sscanf (line,"%[^@]@%[^@]@%[^@]@", ip, date, aphorism);
		list_add_last(list_create_node(ip, aphorism, date), database);
	}
	free(line);
}

void write_to_database(FILE *database, node_t *aph)
{
	fseek(database, 0, SEEK_END);
	fputs(aph->ip, database);
	fputc('@', database);
	fputs(aph->date_time, database);
	fputc('@', database);
	fputs(aph->aphorism, database);
	fputc('@', database);
	fputc('\n', database);
	fflush(database);
}

int server(char *port)
{
	FILE *database = fopen("database", "a+");

	srand(time(NULL));
	linked_list_t *list_aphorisms = (linked_list_t*)malloc(sizeof(linked_list_t));
	list_init(list_aphorisms);
	load_database(database, list_aphorisms);

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
						list_print(list_aphorisms);
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
			char recvbuffer[512];
			int size = 512;
			struct sockaddr_storage client_addr;
			socklen_t address_len = sizeof(client_addr);

			int error = recvfrom(listening_socket, recvbuffer, size, 0, (struct sockaddr*)&client_addr, &address_len);
			if (error < 0){
				printf("[ERROR] Network error\n");
				continue;
			}
			printf("%s\n", recvbuffer);
			printf("%d\n", error);
			if(!strncmp(recvbuffer, "ADD", 3)){
				int msg_length = strnlen(recvbuffer + 3, 509);
				if (msg_length >= 10){
					//recvbuffer[msg_length < 509 ? msg_length : 508] = '\0';
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					if (getnameinfo((struct sockaddr*)&client_addr, address_len, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV) == 0){
						time_t date = time(NULL);
						char *string_date_time = ctime(&date);
						string_date_time[strlen(string_date_time) - 1] = '\0';
						node_t *new_aphorism = list_create_node(hbuf, recvbuffer + 3, string_date_time);
						list_add_last(new_aphorism, list_aphorisms);
						write_to_database(database, new_aphorism);
						printf("[ADD] %s %s\n", hbuf, recvbuffer + 3);
						send_AOK(listening_socket, (struct sockaddr*)&client_addr, address_len);
					}else{
						printf("A LA MIERDA\n");
					}
				}else{
					char error_str[] = "Rejecting aphorism of size < 10";
					printf("[ERROR] %s: %s\n", error_str, recvbuffer + 3);
					send_ERR(listening_socket, (struct sockaddr*)&client_addr, address_len, error_str);
				}
			}else if(!strncmp(recvbuffer, "GET", 3)){
				if (list_aphorisms->count){
					//int send_APH(int socket, struct sockaddr *addr, socklen_t address_len, char *msg);
					send_APH(listening_socket,
						(struct sockaddr*)&client_addr,
						address_len,
						list_get_node_by_index(rand() % list_aphorisms->count, list_aphorisms)->aphorism);
				}else{
					printf("[ERROR] No aphorisms in the database\n");
					char error_str[] = "No aphorisms\0";
					send_ERR(listening_socket, (struct sockaddr*)&client_addr, address_len, error_str);
				}
			}else{
				printf("[ERROR] Unkown command\n");
				char error_str[] = "Unkown command\0";
				send_ERR(listening_socket, (struct sockaddr*)&client_addr, address_len, error_str);
			}
			list_print(list_aphorisms);

		}
		//reset the set
		FD_ZERO(&descriptors_set);
		//add all the descriptors to it
		FD_SET(STDIN_FILENO, &descriptors_set);
		FD_SET(listening_socket, &descriptors_set);
		//also set the max descriptor
		max_fd = listening_socket;
	}
	list_delete(list_aphorisms);
	free(list_aphorisms);
	//at the end, disconnect everyone, clear all.
	close(listening_socket);
	fclose(database);
	return 0;
}

//returns the listening socket
int prepare_server (char *port)
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

	freeaddrinfo(res);
    //the next of the last, NULL -> all failed
    if (ptr == NULL){
    	perror("Bind failed\n");
    	exit (EXIT_FAILURE);
    }

	return socket_descriptor;
}

void help(char *program){
	printf("Server mode: %s -p <listening port>\n", program);
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
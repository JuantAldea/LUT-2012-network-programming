#include "server.h"

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
	linked_list *users = (linked_list*)malloc(sizeof(linked_list));
	list_init(users);
	//get the max number of open files $ ulimit -n
	struct rlimit rlp;

	int MAX_OPENED_FDS = -1;
	if (getrlimit(RLIMIT_NOFILE, &rlp) < 0){
		perror("Error reading the limit of opened descriptors, assumed 1024 by default\n");
		MAX_OPENED_FDS = 1024;
	}else{
		MAX_OPENED_FDS = rlp.rlim_max;
	}

	char *address = NULL;
	int server_socket_descriptor = -1;
	if ((server_socket_descriptor = prepare_server(address, port)) < 0){
		exit(EXIT_FAILURE);
	}

	//wait for connections through the specified socket
	if (listen(server_socket_descriptor, BACKLOG) < 0){
		printf("Error listening %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct sockaddr_storage remote_addr;
	socklen_t addr_size;
	addr_size = sizeof(remote_addr);
	int connection_fd;
	fd_set master_set;
	fd_set ready_set;
	FD_ZERO(&master_set);
	FD_ZERO(&ready_set);
	int number_of_fds = server_socket_descriptor + 1;// fd 0 is stdin, so we have one fd more
	FD_SET(STDIN_FILENO, &master_set);
	FD_SET(server_socket_descriptor, &master_set);

	int running = 1;//server running
	int accepting_new_connections = 1;

	struct timeval select_timeout;//non-blocking select
	memset(&select_timeout, 0, sizeof(struct timeval));

	while(running){
		ready_set = master_set;
		if(select(number_of_fds, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            exit(EXIT_FAILURE);
        }
   		for (int i = 0; i < number_of_fds; i++){
			if (FD_ISSET(i, &ready_set)){
				if (i == STDIN_FILENO){//activity in the standard input
					char command_buffer[10];
					//don't flood me!
					scanf("%9s", command_buffer);
					fflush(stdin);
					int command = parse_command(command_buffer);
					switch(command){
						case SHUTDOWN_COMMAND_CODE:
							running = 0;
							disconnect_clients(users);
							break;
						case START_COMMAND_CODE:
							printf("New connections allowed\n");
							accepting_new_connections = 1;
							break;
						case STOP_COMMAND_CODE:
							printf("New connections disallowed\n");
							accepting_new_connections = 0;
							break;
						case LIST_COMMAND_CODE:
							list_print(users);
							break;
					}
				}else if(i == server_socket_descriptor){//activity in the server entry port
					if ((connection_fd = accept(server_socket_descriptor, (struct sockaddr*)&remote_addr, &addr_size)) < 0){
						printf("Error in the incoming connection %s\n", strerror(errno));
					}
					if (accepting_new_connections){
						printf("New connection accepted\n");
						FD_SET(connection_fd, &master_set);
						number_of_fds++;
						number_of_fds = number_of_fds < MAX_OPENED_FDS ? number_of_fds : MAX_OPENED_FDS;
						char unknow_name[1] = {'\0'};
						node * new_user = list_create_node(connection_fd, unknow_name);
						list_add_last(new_user, users);
						//What happens if number_of_fds become greater than the max number of descriptor per process?
						//getrlimit(RLIMIT_NOFILE, ...) //max number of open files, acording to ulimit -n -> 1024 fds
					}else{
						char error_msg[] = "New connections are not allowed";
						send_error(connection_fd, (uchar *)error_msg);
						printf("New connection dropped\n");
						close(connection_fd);
					}
				}else{
					uchar *buffer = NULL;
					uint8_t type = ERROR_MSG;
					int bytes_readed = recv_msg(i, &buffer, &type);
					if (bytes_readed <= 0){
						printf("Client disconnected\n");
						FD_CLR(i, &master_set);
						list_remove_by_fd(i, users);
						close(i);
					}else{
						if(type == CONNECT_MSG){
							//list_set_name(i, (char *)buffer, users);
						}else if(type == QUIT_MSG){
							printf("Average latency %s\n", (char *)buffer);
						}
						free(buffer);
					}

				}
			}
		}
	}

	close(server_socket_descriptor);
	list_delete(users);//clear list
	free(users);
	return 0;
}

int prepare_server (char *address, char *port)
{
	struct addrinfo hints;
	//fill all the fields with zero, just to be sure. Actually it crashed without this.
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; //don't care ipv4 or ipv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; //automatic self ip
	hints.ai_protocol = IPPROTO_TCP;
	int error = 0;
	struct addrinfo *res = NULL;
	if ((error = getaddrinfo(address, port, &hints, &res)) < 0){
		perror(gai_strerror(error));
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

void disconnect_clients(linked_list *list)
{
	for(node *i = list->head->next; i != list->tail; i = i->next){
		if (close(i->fd) < 0){
			fprintf(stderr, "Error closing fd of %s, ", i->name);
			strerror(errno);
		}
	}
}
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

	int server_socket_descriptor = -1;
	if ((server_socket_descriptor = prepare_server(port)) < 0){
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

	fd_set ready_set;
	FD_ZERO(&ready_set);
	FD_SET(STDIN_FILENO, &ready_set);
	FD_SET(server_socket_descriptor, &ready_set);

	int max_fd = server_socket_descriptor;
	int running = 1;//server running
	int accepting_new_connections = 1;

	while(running){
		if(select(max_fd + 1, &ready_set, NULL, NULL, NULL) < 0) {
			perror("Error in select");
			exit(EXIT_FAILURE);
		}

		if (FD_ISSET(STDIN_FILENO, &ready_set)){
			char command_buffer[10];
			memset(command_buffer, '\0', 10);
			scanf("%9s", command_buffer);
			fflush(stdin);
			int command = parse_command(command_buffer);
			switch(command){
				case SHUTDOWN_COMMAND_CODE:
					running = 0;
					break;
				case START_COMMAND_CODE:
					printf("[SERVER] New connections allowed\n");
					accepting_new_connections = 1;
					break;
				case STOP_COMMAND_CODE:
					printf("[SERVER] New connections disallowed\n");
					accepting_new_connections = 0;
					break;
				case LIST_COMMAND_CODE:
					list_print(users);
					break;
			}
		}

		if (FD_ISSET(server_socket_descriptor, &ready_set)){
			int connection_fd = -1;
			if ((connection_fd = accept(server_socket_descriptor, (struct sockaddr*)&remote_addr, &addr_size)) < 0){
				printf("[SERVER] Error in the incoming connection %s\n", strerror(errno));
			}
			if (accepting_new_connections){
				printf("[SERVER] New client connected with fd = %d\n", connection_fd);
				char unknow_name[1] = {'\0'};
				node * new_user = list_create_node(connection_fd, unknow_name);
				list_add_last(new_user, users);
			}else{
				char error_msg[] = "New connections are not allowed";
				send_error(connection_fd, (uchar *)error_msg);
				printf("[SERVER] New connection dropped.\n");
				close(connection_fd);
			}
		}

		for(node *i = users->head->next; i != users->tail; i = i->next){
			if (FD_ISSET(i->fd, &ready_set)){
				uchar *buffer = NULL;
				uint8_t type = ERROR_MSG;
				int bytes_readed = recv_msg(i->fd, &buffer, &type);
				if (bytes_readed <= 0){
					//the current node is about to be removed
					//so next iteration need an ajustment
					node *previous = i->previous;
					manage_disconnect(i->fd, users);
					i = previous;
				}else{
					if(type == CONNECT_MSG){
						manage_nick_change_by_node(i, (char *)buffer, users);
					}else if(type == QUIT_MSG){
						//the same goes for this
						node *previous = i->previous;
						manage_disconnect(i->fd, users);
						i = previous;
					}else{
						printf("%s\n", buffer);
					}
					free(buffer);
				}
			}
		}

		FD_ZERO(&ready_set);
		FD_SET(STDIN_FILENO, &ready_set);
		FD_SET(server_socket_descriptor, &ready_set);
		max_fd = server_socket_descriptor;
		for(node *i = users->head->next; i != users->tail; i = i->next){
			FD_SET(i->fd, &ready_set);
			max_fd = ((i->fd > max_fd) ? i->fd : max_fd);
		}
	}

	// while(running){
	// 	ready_set = master_set;
	// 	if(select(number_of_fds, &ready_set, NULL, NULL, NULL) < 0) {
	// 		perror("Error in select");
	// 		exit(EXIT_FAILURE);
	// 	}
	// 	for (int i = 0; i < number_of_fds; i++){
	// 		if (FD_ISSET(i, &ready_set)){
	// 			if (i == STDIN_FILENO){//activity in the standard input
		// 				char command_buffer[10];
	// 				//don't flood me!
	// 				scanf("%9s", command_buffer);
	// 				fflush(stdin);
	// 				int command = parse_command(command_buffer);
	// 				switch(command){
	// 					case SHUTDOWN_COMMAND_CODE:
	// 						running = 0;
	// 						disconnect_clients(users);
	// 						break;
	// 					case START_COMMAND_CODE:
	// 						printf("New connections allowed\n");
	// 						accepting_new_connections = 1;
	// 						break;
	// 					case STOP_COMMAND_CODE:
	// 						printf("New connections disallowed\n");
	// 						accepting_new_connections = 0;
	// 						break;
	// 					case LIST_COMMAND_CODE:
	// 						list_print(users);
	// 						break;
	// 				}
	// 			}else if(i == server_socket_descriptor){//activity in the server entry port
	// 				if ((connection_fd = accept(server_socket_descriptor, (struct sockaddr*)&remote_addr, &addr_size)) < 0){
	// 					printf("Error in the incoming connection %s\n", strerror(errno));
	// 				}
	// 				if (accepting_new_connections){
	// 					printf("New connection accepted\n");
	// 					FD_SET(connection_fd, &master_set);
	// 					number_of_fds++;
	// 					number_of_fds = number_of_fds < MAX_OPENED_FDS ? number_of_fds : MAX_OPENED_FDS;
	// 					char unknow_name[1] = {'\0'};
	// 					node * new_user = list_create_node(connection_fd, unknow_name);
	// 					list_add_last(new_user, users);
	// 					//What happens if number_of_fds become greater than the max number of descriptor per process?
	// 					//getrlimit(RLIMIT_NOFILE, ...) //max number of open files, acording to ulimit -n -> 1024 fds
	// 				}else{
	// 					char error_msg[] = "New connections are not allowed";
	// 					send_error(connection_fd, (uchar *)error_msg);
	// 					printf("New connection dropped\n");
	// 					close(connection_fd);
	// 				}
	// 			}else{
	// 				uchar *buffer = NULL;
	// 				uint8_t type = ERROR_MSG;
	// 				int bytes_readed = recv_msg(i, &buffer, &type);
	// 				if (bytes_readed <= 0){
	// 					printf("Client disconnected\n");
	// 					FD_CLR(i, &master_set);
	// 					list_remove_by_fd(i, users);
	// 					close(i);
	// 				}else{
	// 					if(type == CONNECT_MSG){
	// 						list_set_name(i, (char *)buffer, users);
	// 					}else if(type == QUIT_MSG){
	// 					}
	// 					free(buffer);
	// 				}
	// 			}
	// 		}
	// 	}
	// }
	disconnect_clients(users);
	close(server_socket_descriptor);
	list_delete(users);//clear list
	free(users);
	return 0;
}

int prepare_server (char *port)
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
	printf("|%s|\n", port);
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

void disconnect_clients(linked_list *list)
{
	for(node *i = list->head->next; i != list->tail; i = i->next){
		if (strnlen(i->name, MAX_NICKNAME_LENGTH)){
			printf("[SERVER] Disconnecting user %s: ", i->name);
		}else{
			printf("[SERVER] Disconnecting unknow user at fd = %d:", i->fd);
		}
		if (close(i->fd) < 0){
			printf(" FAILED!: %s\n", strerror(errno));
		}else{
			printf("OK!\n");
		}
	}
}

node *manage_disconnect(int fd, linked_list *users)
{
	node *user = list_get_node_by_fd(fd, users);
	close(user->fd);
	if (strnlen(user->name, MAX_NICKNAME_LENGTH)){
		printf("[SERVER] %s disconnected\n", user->name);
	}else{
		printf("[SERVER] User connected with fd = %d disconnected\n", user->fd);
	}
	return list_remove_node(user, users);
}

int manage_nick_change_by_fd(int fd, char *name, linked_list *users)
{
	return nick_chage_by_fd(fd, name, users);
}

int manage_nick_change_by_node(node *i, char *name, linked_list *users)
{
	char old_nick[16];
	int old_nick_len = strnlen(i->name, MAX_NICKNAME_LENGTH);

	if (old_nick_len){
		memset(old_nick, '\0', sizeof(char) * 16);
		memcpy(old_nick, i->name, sizeof(char)*old_nick_len);
	}

	int name_changed = nick_change_by_node(i, name, users);
	if(name_changed){
		if (old_nick_len){
			printf("[SERVER] %s changed nick to %s\n", old_nick, name);
		}else{
			printf("[SERVER] Client with fd = %d sets name to %s\n", i->fd, name);
		}
	}
	return name_changed;
}

int nick_chage_by_fd(int fd, char *name, linked_list *users)
{
	for(node *i = users->head->next; i != users->tail; i = i->next){
		if (i->fd == fd){
			return nick_change_by_node(i, name, users);
		}
	}
	return 0;
}

int nick_change_by_node(node *i, char *name, linked_list *users)
{
	if (!strncmp(i->name, name, MAX_NICKNAME_LENGTH)){
		return 0;
	}
	list_set_name_by_node(i, name, users);
	return 1;
}

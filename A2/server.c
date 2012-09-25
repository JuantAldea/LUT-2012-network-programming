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
	printf("[SERVER] Listening for connections on port %s (fd = %d)\n", port, server_socket_descriptor);
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
				node_t *new_user = list_create_node(connection_fd, unknow_name);
				list_add_last(new_user, users);
			}else{
				char error_msg[] = "New connections are not allowed";
				send_error(connection_fd, error_msg);
				printf("[SERVER] New connection dropped.\n");
				close(connection_fd);
			}
		}

		for(node_t *i = users->head->next; i != users->tail; i = i->next){
			if (FD_ISSET(i->client->fd, &ready_set)){
				int full_message = 0;
				int bytes_readed = recv_msg(i->client->fd, i->buffer, &full_message);
				if (bytes_readed <= 0){
					//the current node is about to be removed
					//so next iteration need an small ajustment
					node_t *previous = i->previous;
					broadcast_quit_message(i->client->name, "disconnected (EOF readed)", users);
					manage_disconnect_by_node(i, users, 0);
					i = previous;
				}else if (full_message){
					if (strnlen(i->client->name, MAX_NICKNAME_LENGTH)){
						switch(i->buffer->message_type){
							case CONNECT_MSG:
							{
								char *nickname = NULL;
								char *introduction = NULL;
								split_connect_message(i->buffer, &nickname, &introduction);
								if(manage_nick_change_by_node(i, nickname, users)){
									//valid
								}else{
									//name in use
								}
								free(nickname);
								free(introduction);
							}
								break;
							case QUIT_MSG:
							{
								node_t *previous = i->previous;
								broadcast_quit_message(i->client->name, (char*)i->buffer->buffer, users);
								manage_disconnect_by_node(i, users, 1);
								i = previous;
							}
								break;
							case WHO_REQUEST_MSG:
								printf("[SERVER] %s requested the user list.\n", i->client->name);
								send_user_list(i->client->fd, users);
								break;
							case CHAT_MSG:
								printf("[SERVER] <%s> %s\n", i->client->name, i->buffer->buffer);
								broadcast_chat_message(i->client->name, (char*)i->buffer->buffer, users);
								break;
							default:
								printf("DUMP: %d %d %s\n", i->buffer->message_length,
												 i->buffer->message_type,
												 i->buffer->buffer);
								break;
						}
					}else{
						if(i->buffer->message_type == CONNECT_MSG){
							char *nickname = NULL;
							char *introduction = NULL;
							split_connect_message(i->buffer, &nickname, &introduction);
							if(manage_nick_change_by_node(i, nickname, users)){
								broadcast_connect_message(nickname, introduction, users);
							}else{
								//name in use
							}
							free(nickname);
							free(introduction);
						}
					}
					recv_buffer_reset(i->buffer);
				}else{
					printf("Partial recv\n");
				}
			}
		}

		FD_ZERO(&ready_set);
		FD_SET(STDIN_FILENO, &ready_set);
		FD_SET(server_socket_descriptor, &ready_set);
		max_fd = server_socket_descriptor;
		for(node_t *i = users->head->next; i != users->tail; i = i->next){
			FD_SET(i->client->fd, &ready_set);
			max_fd = ((i->client->fd > max_fd) ? i->client->fd : max_fd);
		}
	}

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

void disconnect_clients(linked_list_t *list)
{
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		if (strnlen(i->client->name, MAX_NICKNAME_LENGTH)){
			printf("[SERVER] Disconnecting user %s: ", i->client->name);
		}else{
			printf("[SERVER] Disconnecting unknow user at fd = %d:", i->client->fd);
		}
		if (close(i->client->fd) < 0){
			printf(" FAILED!: %s\n", strerror(errno));
		}else{
			printf("OK!\n");
		}
	}
}

void manage_disconnect_by_node(node_t *user, linked_list_t *users, int polite)
{
	close(user->client->fd);
	if (strnlen(user->client->name, MAX_NICKNAME_LENGTH)){
		if (polite){
			printf("[SERVER] %s disconnected\n", user->client->name);
		}else{
			printf("[SERVER] %s disconnected (EOF readed)\n", user->client->name);
		}
	}else{
		if(polite){
			printf("[SERVER] User connected with fd = %d disconnected\n", user->client->fd);
		}else{
			printf("[SERVER] fd = %d: EOF\n", user->client->fd);
		}
	}
	list_remove_node(user, users);
}

int manage_nick_change_by_fd(int fd, char *name, linked_list_t *users)
{
	return nick_chage_by_fd(fd, name, users);
}

int manage_nick_change_by_node(node_t *i, char *name, linked_list_t *users)
{
	char old_nick[16];
	int old_nick_len = strnlen(i->client->name, MAX_NICKNAME_LENGTH);

	if (old_nick_len){
		memset(old_nick, '\0', sizeof(char) * (MAX_NICKNAME_LENGTH + 1));
		memcpy(old_nick, i->client->name, sizeof(char)*MAX_NICKNAME_LENGTH);
	}

	int name_changed = nick_change_by_node(i, name, users);
	if(name_changed){
		if (old_nick_len){
			printf("[SERVER] %s changed nick to %s\n", old_nick, name);
			broadcast_nickname_change(old_nick, name, users);
		}else{
			printf("[SERVER] Client with fd = %d sets name to %s\n", i->client->fd, name);
		}
	}
	return name_changed;
}

int nick_chage_by_fd(int fd, char *name, linked_list_t *users)
{
	for(node_t *i = users->head->next; i != users->tail; i = i->next){
		if (i->client->fd == fd){
			return nick_change_by_node(i, name, users);
		}
	}
	return 0;
}

int nick_change_by_node(node_t *i, char *name, linked_list_t *users)
{
	if (!strncmp(i->client->name, name, MAX_NICKNAME_LENGTH)){
		return 0;
	}
	list_set_name_by_node(i, name, users);
	return 1;
}

void split_connect_message(recv_buffer_t *buffer, char **nickname, char **introduction)
{
	int introduction_length = strnlen((char *)(buffer->buffer + MAX_NICKNAME_LENGTH),
										 buffer->message_length - MAX_NICKNAME_LENGTH);
	//int nickname_length     = strnlen((char *) buffer->buffer, MAX_NICKNAME_LENGTH);

	*introduction = (char*)malloc(sizeof(uchar) * (introduction_length + 1));
	*nickname     = (char*)malloc(sizeof(uchar) * (MAX_NICKNAME_LENGTH + 1));

	memset((uchar*)*introduction, '\0', sizeof(uchar) * (introduction_length + 1));
	memset((uchar*)*nickname,     '\0', sizeof(uchar) * (MAX_NICKNAME_LENGTH + 1));

	memcpy(*introduction, buffer->buffer + sizeof(uchar) * MAX_NICKNAME_LENGTH,
		sizeof(uchar) * introduction_length);
	memcpy(*nickname, 	  buffer->buffer, sizeof(uchar) * MAX_NICKNAME_LENGTH);
}

void broadcast_chat_message(char *nickname, char *message, linked_list_t *users)
{
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		int sent_bytes = send_fwd_chat_msg(i->client->fd, nickname, message);
	}
}

void broadcast_quit_message(char *nickname, char *message, linked_list_t *users)
{
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		int sent_bytes = send_fwd_client_left(i->client->fd, nickname, message);
	}
}

void broadcast_nickname_change(char *oldname, char *newname, linked_list_t *users)
{
	char base_message[] = "changed name to";
	int message_length = MAX_NICKNAME_LENGTH * 2 + strlen(base_message) + 3;
	char *message = (char*)malloc(sizeof(char) * message_length);
	memset(message, '\0', message_length);
	sprintf(message, "%s %s %s", oldname, base_message, newname);
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		send_accept_nickname_change(i->client->fd, message);
	}
	free(message);
}

void broadcast_connect_message(char *name, char *introduction, linked_list_t *users)
{
	char base_message[] = "connected:";
	printf("[SERVER] Broadcasting introduction: <%s> %s\n", name, introduction);
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		send_fwd_introduction_msg(i->client->fd, name, introduction);
	}
}

/*
###############################################
#		CT30A5001 - Network Programming		  #
#		Assignment2: TCP multiuser chat		  #
# 	Juan Antonio Aldea Armenteros (0404450)   #
# 		juan.aldea.armenteros@lut.fi		  #
#					server.c				  #
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

	//wait for connections through the specified socket
	if (listen(listening_socket, BACKLOG) < 0){
		printf("Error listening %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	struct sockaddr_storage remote_addr;
	socklen_t addr_size;
	addr_size = sizeof(remote_addr);

	fd_set descriptors_set;
	FD_ZERO(&descriptors_set);
	FD_SET(STDIN_FILENO, &descriptors_set);
	FD_SET(listening_socket, &descriptors_set);

	int max_fd = listening_socket;
	int running = 1;//server running
	int accepting_new_connections = 1;
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
			//we have an incomming connection
			int connection_fd = -1;
			if ((connection_fd = accept(listening_socket, (struct sockaddr*)&remote_addr, &addr_size)) < 0){
				printf("[NEW CONNECTION] Error in the incoming connection %s\n", strerror(errno));
			}
			//if we are accepting new connections, register the new user
			if (accepting_new_connections){
				printf("[NEW CONNECTION] New client connected with fd = %d\n", connection_fd);
				char unknow_name[1] = {'\0'};
				node_t *new_user = list_create_node(connection_fd, unknow_name);
				list_add_last(new_user, users);
			}else{
				//if we are not, drop the connection.
				char error_msg[] = "New connections are not allowed";
				send_error(connection_fd, error_msg);
				printf("[NEW CONNECTION] New connection dropped.\n");
				close(connection_fd);
			}
		}
		//check activity in every client descriptor
		for(node_t *i = users->head->next; i != users->tail; i = i->next){
			if (FD_ISSET(i->client->fd, &descriptors_set)){
				//if we read, we need to know if we received the whole msg
				int full_message = 0;
				int bytes_readed = recv_msg(i->client->fd, i->buffer, &full_message);
				//if we read 0 bytes, we have a disconnect
				if (bytes_readed <= 0){
					//the current node is about to be removed
					//so next iteration need an small ajustment
					node_t *previous = i->previous;
					broadcast_quit_message(i->client->name, "disconnected (EOF readed)", users);
					manage_disconnect_by_node(i, users, 0);
					i = previous;
				}else if (full_message){//we received the whole msg
					//check if the client is already registered
					if (strnlen(i->client->name, MAX_NICKNAME_LENGTH)){
						//look at the msg type
						switch(i->buffer->message_type){
							case CONNECT_MSG:
							{
								//the client is already registered and we got a connect msg
								// so is a nickname change request
								char *nickname = NULL;
								char *introduction = NULL;
								split_connect_message(i->buffer, &nickname, &introduction);
								//try to change the nick
								if(!manage_nick_change_by_node(i, nickname, users)){
									send_error(i->client->fd, "Nickname is already in use");
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
								printf("[USER LIST] %s requested the user list.\n", i->client->name);
								send_user_list(i->client->fd, users);
								break;
							case CHAT_MSG:
								printf("[CHAT] <%s> %s\n", i->client->name, i->buffer->buffer);
								broadcast_chat_message(i->client->name, (char*)i->buffer->buffer, users);
								break;
							default:
								printf("DUMP: %d %d %s\n", i->buffer->message_length,
												 i->buffer->message_type,
												 i->buffer->buffer);
								break;
						}
					}else{//the length of the nickname is 0, so the client isn't registered
						//if the client is trying to get registered
						if(i->buffer->message_type == CONNECT_MSG){
							char *nickname = NULL;
							char *introduction = NULL;
							split_connect_message(i->buffer, &nickname, &introduction);
							//try to set the nick
							if(manage_nick_change_by_node(i, nickname, users)){
								//if the nick is valid, good news everyone!
								broadcast_connect_message(nickname, introduction, users);
							}else{
								//if the nick is in use, kick the client
								printf("[DROPPING CLIENT] nickname already in use\n");
								send_error(i->client->fd, "Nickname already in use");
								node_t *previous = i->previous;
								manage_disconnect_by_node(i, users, 0);
								i = previous;
							}
							free(nickname);
							free(introduction);
						}
					}
					recv_buffer_reset(i->buffer);
				}else{
					//printf("Partial recv\n");
				}
			}
		}
		//reset the set
		FD_ZERO(&descriptors_set);
		//add all the descriptors to it
		FD_SET(STDIN_FILENO, &descriptors_set);
		FD_SET(listening_socket, &descriptors_set);
		//also set the max descriptor
		max_fd = listening_socket;
		for(node_t *i = users->head->next; i != users->tail; i = i->next){
			FD_SET(i->client->fd, &descriptors_set);
			max_fd = ((i->client->fd > max_fd) ? i->client->fd : max_fd);
		}
	}

	//at the end, disconnect everyone, clear all.
	disconnect_clients(users);
	close(listening_socket);
	list_delete(users);//clear list
	free(users);
	return 0;
}

//returns the listening socket
int prepare_server (char *port)
{
	struct addrinfo hints;
	//fill all the fields with zero, just to be sure. Actually it crashed without this.
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6; //don't care ipv4 or ipv6
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

//disconnect all clients
void disconnect_clients(linked_list_t *list)
{
	for(node_t *i = list->head->next; i != list->tail; i = i->next){
		if (strnlen(i->client->name, MAX_NICKNAME_LENGTH)){
			printf("[SHUTTING DOWN] Disconnecting user %s: ", i->client->name);
		}else{
			printf("[SHUTTING DOWN] Disconnecting unknow user at fd = %d:", i->client->fd);
		}
		if (close(i->client->fd) < 0){
			printf(" FAILED!: %s\n", strerror(errno));
		}else{
			printf(" OK!\n");
		}
	}
}

//disconnect the client stored in the node
void manage_disconnect_by_node(node_t *user, linked_list_t *users, int polite)
{
	close(user->client->fd);
	//if the client was registered
	if (strnlen(user->client->name, MAX_NICKNAME_LENGTH)){
		if (polite){
			printf("[DISCONNECTION] %s disconnected\n", user->client->name);
		}else{
			printf("[DISCONNECTION] %s disconnected (EOF readed)\n", user->client->name);
		}
	}else{//if it wasn't, the name is unknow
		if(polite){
			printf("[DISCONNECTION] User connected with fd = %d disconnected\n", user->client->fd);
		}else{
			printf("[DISCONNECTION] fd = %d: EOF\n", user->client->fd);
		}
	}
	list_remove_node(user, users);
}

//try to change the nick of the given node
int manage_nick_change_by_node(node_t *user, char *name, linked_list_t *users)
{
	char old_nick[MAX_NICKNAME_LENGTH + 1];
	//if the client has a name (is already registered)
	int old_nick_len = strnlen(user->client->name, MAX_NICKNAME_LENGTH);
	if (old_nick_len){
		//save the old name, we will need it to build the message
		memset(old_nick, '\0', sizeof(char) * (MAX_NICKNAME_LENGTH + 1));
		memcpy(old_nick, user->client->name, sizeof(char) * MAX_NICKNAME_LENGTH);
	}
	//try to change the name
	int name_changed = nick_change_by_node(user, name, users);

	if(name_changed){
		//the name is changed, if the client has an old name (if the client was registered)
		if (old_nick_len){
			//broadcast a nick change
			printf("[NICK CHANGE] %s changed nick to %s\n", old_nick, name);
			broadcast_nickname_change(old_nick, name, users);
		}else{
			//the client wasn't registered, so we have a new user
			printf("[NEW LOGIN] Client with fd = %d sets name to %s\n", user->client->fd, name);
		}
	}
	return name_changed;
}

int nick_change_by_node(node_t *node, char *name, linked_list_t *users)
{
	//if the nicks are the same, no change at all (quick test)
	if (!strncmp(node->client->name, name, MAX_NICKNAME_LENGTH)){
		return 0;
	}
	//if are different, check the whole list
	for(node_t *i = users->head->next; i != users->tail; i = i->next){
		if (!strncmp(i->client->name, name, MAX_NICKNAME_LENGTH)){
			return 0;
		}
	}
	//if the name isn't in use, set it
	list_set_name_by_node(node, name, users);
	return 1;
}

//split the buffer into nickname and the introduction
void split_connect_message(recv_buffer_t *buffer, char **nickname, char **introduction)
{
	//since the length of the nickname is fixed, the rest of the message is the introduction
	int introduction_length = buffer->message_length - MAX_NICKNAME_LENGTH;
	//allocate the memory needed
	*introduction = (char*)malloc(sizeof(uchar) * (introduction_length + 1));
	*nickname     = (char*)malloc(sizeof(uchar) * (MAX_NICKNAME_LENGTH + 1));

	memset((uchar*)*introduction, '\0', sizeof(uchar) * (introduction_length + 1));
	memset((uchar*)*nickname,     '\0', sizeof(uchar) * (MAX_NICKNAME_LENGTH + 1));
	//copy the data into the buffers
	memcpy(*nickname, buffer->buffer, sizeof(uchar) * MAX_NICKNAME_LENGTH);
	memcpy(*introduction, buffer->buffer + sizeof(uchar) * MAX_NICKNAME_LENGTH, sizeof(uchar) * introduction_length);
}

//send a chat message to every user
void broadcast_chat_message(char *nickname, char *message, linked_list_t *users)
{
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		send_fwd_chat_msg(i->client->fd, nickname, message);
	}
}

//send a quit message to every user
void broadcast_quit_message(char *nickname, char *message, linked_list_t *users)
{
	printf("[SERVER] Broadcasting quit message from %s: %s\n", nickname, message);
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		send_fwd_client_left(i->client->fd, nickname, message);
	}
}

//send a nickname change message to every user
void broadcast_nickname_change(char *oldname, char *newname, linked_list_t *users)
{
	char base_message[] = "changed name to";
	int message_length = MAX_NICKNAME_LENGTH * 2 + strlen(base_message) + 3;
	char *message = (char*)malloc(sizeof(char) * message_length);
	memset(message, '\0', message_length);
	//the protocol defines a message with only one field, so custom msg is sent
	sprintf(message, "%s %s %s", oldname, base_message, newname);
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		send_accept_nickname_change(i->client->fd, message);
	}
	free(message);
}

//send the connect message to everyone
void broadcast_connect_message(char *name, char *introduction, linked_list_t *users)
{
	printf("[INTRODUCTION] Broadcasting introduction: <%s> %s\n", name, introduction);
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		send_fwd_introduction_msg(i->client->fd, name, introduction);
	}
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

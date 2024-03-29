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

int read_stdin(char **msg)
{
    int bytes_on_stdin;
    ioctl(STDIN_FILENO, FIONREAD, &bytes_on_stdin);
    char *buffer = NULL;
    buffer = (char*)malloc(sizeof(char) * (bytes_on_stdin + 1));
    if (fgets(buffer, bytes_on_stdin, stdin) == NULL){
    	printf("Error while reading from STDIN\n");
    }
    fgetc(stdin);//read newline
    if(!strlen(buffer)){
        *msg = (char*)malloc(sizeof(char));
        *msg[0] = '\0';
        return UNKOWN_COMMAND_CODE;
    }

    if (buffer[0] == '/'){
        int command_code = UNKOWN_COMMAND_CODE;
        if(!strncmp(&buffer[1], QUIT_COMMAND, strlen(QUIT_COMMAND))){
            command_code = QUIT_COMMAND_CODE;
        }else if(!strncmp(&buffer[1], NICK_COMMAND, strlen(NICK_COMMAND))){
            command_code = NICK_COMMAND_CODE;
        }else if(!strncmp(&buffer[1], CONNECT_COMMAND, strlen(CONNECT_COMMAND))){
            command_code = CONNECT_COMMAND_CODE;
        }else if(!strncmp(&buffer[1], WHO_COMMAND, strlen(WHO_COMMAND))){
            command_code = WHO_COMMAND_CODE;
        }
        if(command_code != UNKOWN_COMMAND_CODE && command_code != WHO_COMMAND_CODE){
            size_t first_white_space_index = strcspn(buffer, " ");
            if (buffer[first_white_space_index] != '\0'){
                int message_length = bytes_on_stdin - first_white_space_index; //-1+1 space and \0
                *msg = (char*)malloc(sizeof(char) * message_length);
                memset(*msg, '\0', message_length);
                sprintf(*msg, "%s", &buffer[first_white_space_index + 1]);
            }else{
                *msg = (char*)malloc(sizeof(char));
                *msg[0] = '\0';
            }
        }else{
            *msg = (char*)malloc(sizeof(char));
            *msg[0] = '\0';
        }

        free(buffer);
        return command_code;
    }else{//message
        *msg = buffer;
        return CHAT_COMMAND_CODE;
    }
    *msg = NULL;
    return UNKOWN_COMMAND_CODE;
}

int client(char *name, char *address, char *port)
{
	fd_set ready_set;
	FD_ZERO(&ready_set);
	int not_connected = 1;
	int socket_descriptor = -1;
	//poll the stdin for the /connect command
	while (not_connected){
		FD_SET(STDIN_FILENO, &ready_set);
		if(select(STDIN_FILENO + 1, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            return EXIT_FAILURE;
        }
        if (FD_ISSET(STDIN_FILENO, &ready_set)){
        	char *message = NULL;
			int command = read_stdin(&message);
			switch(command){
				case QUIT_COMMAND_CODE:
					free(message);
					return 0;
					break;
				case CONNECT_COMMAND_CODE:
					if ((socket_descriptor = prepare_connection(address, port)) < 0){
						free(message);
						return EXIT_FAILURE;
					}
					send_login(socket_descriptor, name, message);
					not_connected = 0;
					break;
				default:
					printf("Not Connected.\n");
					break;
			}
			free(message);
        }
	}


	int number_of_fds = socket_descriptor + 1;
	int running = 1;
	//we are connected, no we will listen to stdin and the connection socket
	while(running){
		FD_SET(STDIN_FILENO, &ready_set);
		FD_SET(socket_descriptor, &ready_set);
		//select descriptors with activity
		if(select(number_of_fds, &ready_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            return EXIT_FAILURE;
        }
        //activity in stdin
		if (FD_ISSET(STDIN_FILENO, &ready_set)){
			//parse the command
			char *message = NULL;
			int command = read_stdin(&message);
			//and do what you have to do
			switch(command){
				case QUIT_COMMAND_CODE:
					send_disconnect(socket_descriptor, message);
					running = 0;
					break;
				case WHO_COMMAND_CODE:
					send_who_request(socket_descriptor);
					break;
				case CHAT_COMMAND_CODE:
					send_chat(socket_descriptor, message);
					break;
				case NICK_COMMAND_CODE:
					send_login(socket_descriptor, message, "");
					break;
				default:
					printf("[ERROR] UNKOWN COMMAND\n");
					break;
			}
			free(message);
		}
		//activity in the socket
		if(FD_ISSET(socket_descriptor, &ready_set)){
			recv_buffer_t *buffer = NULL;
			buffer = (recv_buffer_t*)malloc(sizeof(recv_buffer_t));
			memset(buffer, 0, sizeof(recv_buffer_t));
			recv_buffer_reset(buffer);
			int full_message = 0;
			int recv_bytes = recv_msg(socket_descriptor, buffer, &full_message);
			if (recv_bytes <= 0){
				printf("Connection to the server lost\n");
				running = 0;
			}else{
				print_message(buffer);
			}
			recv_buffer_free(buffer);
		}
	}

	printf("Quitting...(if you are running valgrind you may see an invalid free() in the glibc (2.16) scope)\n");
	close(socket_descriptor);
	return EXIT_SUCCESS;
}

//returns the socket descriptor of the
int prepare_connection(char *address, char *port)
{
	struct addrinfo hints;
	//fill all the fields with zero, just to be sure. Actually it crashed without this.
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6; //only IPv6 allowed
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
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

void print_chat_message(recv_buffer_t *buffer)
{
	char *name = strndup((char*)buffer->buffer, MAX_NICKNAME_LENGTH);
	printf("<%s> %s\n", name, &buffer->buffer[MAX_NICKNAME_LENGTH]);
	free(name);
}

void print_introduction_message(recv_buffer_t *buffer)
{
	if (strlen((char*)&buffer->buffer[MAX_NICKNAME_LENGTH])){
		printf("\n[CONNECTED] <%s>: %s\n", (char*)buffer->buffer, (char*)&buffer->buffer[MAX_NICKNAME_LENGTH]);
	}else{
		printf("\n[CONNECTED] %s\n", (char*)buffer->buffer);
	}
}

void print_user_list(recv_buffer_t *buffer)
{
	printf ("#################################### USER LIST ####################################\n");
	int buffer_offset = 0;
	while (buffer_offset < buffer->message_length){
		printf("%s\n", buffer->buffer + buffer_offset);
		buffer_offset += strnlen(((char*)buffer->buffer) + buffer_offset, MAX_NICKNAME_LENGTH) + 1;//count \0
	}
	printf ("###################################################################################\n");
}

void print_client_left(recv_buffer_t *buffer)
{
	//if the client who left has said something
	if (buffer->message_length > MAX_NICKNAME_LENGTH){
		printf("[DISCONNECTED] %s: %s\n", buffer->buffer, (buffer->buffer + MAX_NICKNAME_LENGTH));
	}else{
		printf("[DISCONNECTED] %s\n", buffer->buffer);
	}

}

void print_nickname_change(recv_buffer_t *buffer)
{
	printf("[NICK CHANGE] %s\n", buffer->buffer);
}

void print_error_msg(recv_buffer_t * buffer)
{
	printf("[ERROR] %s\n", buffer->buffer);
}

void print_message(recv_buffer_t *buffer)
{
	if(buffer->message_type == INTRODUCTION_FWD_MSG){
		print_introduction_message(buffer);
	}else if(buffer->message_type == ACCEPT_NICKNAME_MSG){
		print_nickname_change(buffer);
	}else if (buffer->message_type == CHAT_FWD_MSG){
		print_chat_message(buffer);
	}else if(buffer->message_type == CLIENT_LIST_MSG){
		print_user_list(buffer);
	}else if(buffer->message_type == CLIENT_LEFT_MSG){
		print_client_left(buffer);
	}else if(buffer->message_type == ERROR_MSG){
		print_error_msg(buffer);
	}else{
		printf("DUMP: |%s|\n", buffer->buffer);
	}
}

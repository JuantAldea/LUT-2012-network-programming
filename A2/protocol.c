#include "protocol.h"

//wrap and send the msg by the socket
int send_msg(int socket, uchar *msg, int msg_size, int16_t type)
{
	//allocate the needed buffer
	uint32_t wrapped_size = msg_size + sizeof(int32_t) + sizeof(int16_t);
	uchar * wrapped_msg = (uchar *) malloc(sizeof(uchar) * wrapped_size);
	memset(wrapped_msg, '\0', sizeof(uchar) * wrapped_size);
	//set the length of the msg
	*(int32_t *)wrapped_msg = htonl(wrapped_size);
	//set the type
	*(int16_t *)(wrapped_msg + sizeof(int32_t)) = htons(type);
	//set the payload
	memcpy(wrapped_msg + sizeof(int32_t) + sizeof(int16_t), msg, sizeof(uchar) * msg_size);

	//send the whole msg
	int bytes_to_send = sizeof(uchar) * wrapped_size;
	int total_sent_bytes = 0;
	while (total_sent_bytes < bytes_to_send){
		total_sent_bytes += send(socket, &(wrapped_msg[total_sent_bytes]), bytes_to_send - total_sent_bytes, 0);
	}
	free(wrapped_msg);
	return total_sent_bytes;
}

// returns in full_message by reference if the whole message has been received
int recv_msg(int socket, recv_buffer_t *buffer, int *full_message)
{
	int recv_bytes = -1;
	int bytes_availables = -1;
	//check the bytes availables in the socket
	ioctl(socket, FIONREAD, &bytes_availables);

	//received 0 bytes -> disconnect
	if (bytes_availables == 0){
		*full_message = 1;
		buffer->message_length = 0;
		buffer->received_bytes = 0;
		return 0;
	}

	//we dont have the length header yet, we need it (is the first read of the msg)
	if (buffer->received_bytes == 0){
		//if we dont have enough bytes for the header
		if (bytes_availables < HEADER_SIZE){
			*full_message = 0;
			buffer->message_length = -1;
			buffer->received_bytes = 0;
			//just return
			return -2;
		}
		//we have at least the size of the header
		//so we can at least init the buffer and store the length of the message
		recv_bytes = recv(socket, &buffer->message_length, sizeof(int32_t), 0);
		recv_bytes = recv(socket, &buffer->message_type, sizeof(int16_t), 0);
		//store the sice of the payload, no the whole message
		buffer->message_length = ntohl(buffer->message_length) - HEADER_SIZE;
		//store the type
		buffer->message_type   = ntohs(buffer->message_type);
		//allocate the buffer
		buffer->buffer = (uchar *)malloc(sizeof(uchar) * buffer->message_length + 1);
		//\0 it
		memset(buffer->buffer, '\0', buffer->message_length + 1);
		//since we have read, available bytes needs an update
		ioctl(socket, FIONREAD, &bytes_availables);
	}

	//we have more or less than we need?
	int32_t remaining_bytes = buffer->message_length - buffer->received_bytes;
	size_t recv_size = (remaining_bytes > bytes_availables ? bytes_availables : remaining_bytes);
	//read all that we have/need
	recv_bytes = recv(socket, &(buffer->buffer[buffer->received_bytes]), recv_size, 0);
	buffer->received_bytes += recv_bytes;
	//if we have all what we need, the the message is fully readed
	if (buffer->message_length == buffer->received_bytes){
		*full_message = 1;
	}else{
		*full_message = 0;
	}
	return buffer->received_bytes; //what we have received so far
}

//build the login package and send it
int send_login(int socket, char *username, char *introduction_message)
{
	int32_t username_length = strnlen(username, MAX_NICKNAME_LENGTH);
	int32_t introduction_message_length = strlen(introduction_message);
	//fixed nickname size, worst case
	int32_t buffer_length = introduction_message_length + MAX_NICKNAME_LENGTH;
	//allocate the buffer for the message
	uchar *buffer = (uchar *)malloc(sizeof(uchar) * buffer_length);
	memset(buffer, '\0', sizeof(uchar) * buffer_length);
	//fill it
	memcpy(buffer, username, sizeof(char) * username_length);
	memcpy(((char*)buffer) + MAX_NICKNAME_LENGTH, introduction_message, sizeof(char) * introduction_message_length);
	//and send it
	int bytes_sent = send_msg(socket, buffer, buffer_length, CONNECT_MSG);
	free(buffer);
	return bytes_sent;
}

// client to server

int send_chat(int socket, char *msg)
{
	return send_msg(socket, (uchar *)msg, strlen(msg) + 1, CHAT_MSG);
}

int send_who_request(int socket)
{
	char zero = '\0';
	return send_msg(socket, (uchar *)&zero, 1, WHO_REQUEST_MSG);
}

int send_disconnect(int socket, char *quit_message)
{
	return send_msg(socket, (uchar *)quit_message, strlen(quit_message) + 1, QUIT_MSG);
}

//SERVER TO CLIENT
//build the forwarded introduction message and send it
int send_fwd_introduction_msg(int socket, char *nickname, char *msg)
{
	//allocate the buffer
	int msg_length = strlen(msg);
	int buffer_size = MAX_NICKNAME_LENGTH + msg_length;
	char *buffer = (char *)malloc(sizeof(char) * buffer_size);
	memset(buffer, '\0', sizeof(char) * buffer_size);
	//fill it
	memcpy(buffer, nickname, sizeof(char) * MAX_NICKNAME_LENGTH);
	memcpy(&buffer[MAX_NICKNAME_LENGTH], msg, msg_length);
	//send it
	int sent_bytes = send_msg(socket, (uchar *)buffer, buffer_size, INTRODUCTION_FWD_MSG);
	free(buffer);
	return sent_bytes;
}

int send_accept_nickname_change(int socket, char *message)
{
	return send_msg(socket, (uchar *)message, strlen(message) + 1, ACCEPT_NICKNAME_MSG);
}

int send_fwd_chat_msg(int socket, char *nickname, char *msg)
{
	int msg_length = strlen(msg);
	int buffer_length = MAX_NICKNAME_LENGTH + msg_length;
	char *buffer = (char *)malloc(sizeof(char) * buffer_length);
	memset(buffer, '\0', sizeof(char) * buffer_length);
	memcpy(buffer, nickname, strnlen(nickname, MAX_NICKNAME_LENGTH));
	memcpy(buffer + MAX_NICKNAME_LENGTH, msg, msg_length);
	int sent_bytes = send_msg(socket, (uchar *)buffer, buffer_length, CHAT_FWD_MSG);
	free(buffer);
	return sent_bytes;
}

int send_user_list(int socket, linked_list_t *users)
{
	//worst case: all nicknames have 15 chars and one extra char (\0) for each name
	//so we need a buffer suitable for the worst case scenario
	//allocate it
	int buffer_size = sizeof(char) *(MAX_NICKNAME_LENGTH * users->count + users->count);
	char *list = (char *)malloc(buffer_size);
	memset(list, '\0', buffer_size);
	//pointer to the first not used position of the list
	char *list_offset = list;
	int list_length = 0;
	//fill the  buffer
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		int current_name_length = strnlen(i->client->name, MAX_NICKNAME_LENGTH);
		list_length += current_name_length + 1;
		memcpy(list_offset, i->client->name, sizeof(char)  * current_name_length);
		list_offset += current_name_length + 1;
	}
	int sent_bytes = send_msg(socket, (uchar *)list, list_length, CLIENT_LIST_MSG);
	free(list);
	return sent_bytes;
}

int send_fwd_client_left(int socket, char *nickname, char *quit_message)
{
	//allocate the buffer
	int quit_message_length = strlen(quit_message);
	int message_length = quit_message_length + MAX_NICKNAME_LENGTH;
	char *buffer = (char *)malloc(sizeof(char) * message_length);
	memset(buffer, '\0', message_length);
	//fill the buffer
	memcpy(buffer, nickname, MAX_NICKNAME_LENGTH);
	memcpy(buffer + MAX_NICKNAME_LENGTH, quit_message, quit_message_length);
	//send the buffer
	int sent_bytes = send_msg(socket, (uchar *)buffer, message_length, CLIENT_LEFT_MSG);
	free(buffer);
	return sent_bytes;
}

int send_error(int socket, char *msg)
{
	return send_msg(socket, (uchar *)msg, strlen(msg) + 1, ERROR_MSG);
}

//print the contents of the buffer, changing \0 by -
void buffer_dump(char * buffer, int length)
{
	for (int j = 0; j < length; j++){
		if (buffer[j] == '\0'){
			printf("-");
		}else{
			printf("%c", (char)buffer[j]);
		}
	}
	printf("\n");
}

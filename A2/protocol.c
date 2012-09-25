#include "protocol.h"

int send_msg(int socket, uchar *msg, int msg_size, int16_t type)
{
	//uint32_t msg_size = strlen((char *)msg) + 1;// add \0 => + 1
	uint32_t wrapped_size = msg_size + sizeof(int32_t) + sizeof(int16_t);
	uchar * wrapped_msg = (uchar *) malloc(sizeof(uchar) * wrapped_size);
	memset(wrapped_msg, '\0', sizeof(uchar) * wrapped_size);

	*(int32_t *)wrapped_msg = htonl(wrapped_size);
	*(int16_t *)(wrapped_msg + sizeof(int32_t)) = htons(type);
	memcpy((void *)(wrapped_msg + sizeof(int32_t) + sizeof(int16_t)), msg, sizeof(uchar)* msg_size);

	//printf("Send %d\n", *(int32_t*)(wrapped_msg));
	//printf("Send %d\n", *(int16_t*)(wrapped_msg + sizeof(int32_t)));
	//printf("Send %s\n",  (char   *)(wrapped_msg + sizeof(int32_t) + sizeof(int16_t)));

	int bytes_to_send = wrapped_size * sizeof(uchar);
	int total_sent_bytes = 0;
	while (total_sent_bytes < bytes_to_send){
		total_sent_bytes += send(socket, &(wrapped_msg[total_sent_bytes]), bytes_to_send - total_sent_bytes, 0);
	}
	printf("############DUMP en enviar:");
	for (int j =  sizeof(int32_t) + sizeof(int16_t); j < bytes_to_send; j++){
		if (wrapped_msg[j] == '\0'){
			printf("-");
		}else{
			printf("%c", (char)wrapped_msg[j]);
		}
	}
	printf("\n");
	free(wrapped_msg);
	return total_sent_bytes;
}

int recv_msg(int socket, recv_buffer_t *buffer, int *full_message)
{
	int recv_bytes = -1;
	int bytes_availables = -1;
	ioctl(socket, FIONREAD, &bytes_availables);
	//todo check ioctl
	if (buffer->received_bytes == 0){//we dont have the length header yet
		if (bytes_availables < 6){//sizeof(int32_t)){//we dont have enough bytes for the header
			*full_message = 0;
			buffer->message_length = -1;
			buffer->received_bytes = 0;
			return -2;//just return
		}
		//we dont have header but we have at least the size of the header
		//so we can at least init the buffer and store the length of the message
		recv_bytes = recv(socket, &buffer->message_length, sizeof(int32_t), 0);
		recv_bytes = recv(socket, &buffer->message_type, sizeof(int16_t), 0);
		buffer->message_length = ntohl(buffer->message_length) - 6;
		buffer->message_type   = ntohs(buffer->message_type);
		buffer->buffer = (uchar *)malloc(sizeof(uchar) * buffer->message_length);
		ioctl(socket, FIONREAD, &bytes_availables);//update
	}

	//received 0 bytes -> disconnect
	if (bytes_availables == 0){
		*full_message = 1;
		buffer->message_length = 0;
		buffer->received_bytes = 0;
		return 0;
	}

	//printf("%d\n", buffer->message_length);
	//printf("%d\n", buffer->message_type);
	//printf("%d\n", buffer->received_bytes);
	//we have more or less than we need?
	int32_t remaining_bytes = buffer->message_length - buffer->received_bytes;
	size_t recv_size = (remaining_bytes > bytes_availables ? bytes_availables : remaining_bytes);
	//read all that we have/need
	recv_bytes = recv(socket, &(buffer->buffer[buffer->received_bytes]), recv_size, 0);
	buffer->received_bytes += recv_bytes;
	if (buffer->message_length == buffer->received_bytes){
		*full_message = 1;
		printf("DUMP en recibir:");
		for (int j = 0; j < buffer->message_length; j++){
			if (buffer->buffer[j] == '\0'){
				printf("-");
			}else{
				printf("%c", (char)buffer->buffer[j]);
			}
		}
		printf("\n");
	}else{
		*full_message = 0;
	}
	return buffer->received_bytes; //what we have received so far
}

int send_login(int socket, char *username, char *introduction_message)
{
	uint32_t msg_length = 0;
	uint32_t username_length = 0;
	username_length = strnlen(username, 15);
	msg_length = strlen(introduction_message) + 1; //add a free \0
	int32_t buffer_size = msg_length + 15;//fixed nickname size, worst case

	uchar *buffer = (uchar *)malloc(sizeof(uchar) * buffer_size);
	memset(buffer, '\0', sizeof(uchar) * buffer_size);// 0 fill
	buffer[buffer_size-1] = '\0';
	memcpy(buffer, username, sizeof(uchar) * username_length);
	memcpy(&(((char*)buffer)[15]), introduction_message, sizeof(uchar) * msg_length);
	printf("Dump en send_login:");
	for (int j = 0; j < buffer_size; j++){
		if (buffer[j] == '\0'){
			printf("-");
		}else{
			printf("%c", (char)buffer[j]);
		}
	}
	printf("\n");
	printf("Login size %d\n", buffer_size);
	int bytes_sent = send_msg(socket, buffer, buffer_size, CONNECT_MSG);
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
	//return send_msg(socket, (uchar *)&zero, WHO_REQUEST_MSG);
}

int send_disconnect(int socket, char *quit_message)
{
	//return send_msg(socket, (uchar *)quit_message, QUIT_MSG);
}

//SERVER TO CLIENT

int send_fwd_introduction_msg(int socket, char *nickname, char *msg)
{
	printf("111111111111111%c\n", nickname[14]);
	int msg_length = strlen(msg);
	int buffer_size = 15 + msg_length + 1;
	char *buffer = (char*)malloc(sizeof(char) * buffer_size);
	memset(buffer, '\0', sizeof(char) * buffer_size);
	memcpy(buffer, nickname, sizeof(char) * 15);
	memcpy(&buffer[15], msg, msg_length);
	int sent_bytes = send_msg(socket, (uchar *)buffer, buffer_size, INTRODUCTION_FWD_MSG);
	free(buffer);
	return sent_bytes;
}

int send_accept_nickname_change(int socket, char *message)
{
	printf("Accept nick change |%s|\n", message);
	return send_msg(socket, (uchar *)message, strlen(message) + 1, ACCEPT_NICKNAME_MSG);
}

int send_fwd_chat_msg(int socket, char *nickname, char *msg)
{
	int msg_length = strlen(msg);
	int buffer_length = 15 + msg_length + 1;
	char *buffer = (char *)malloc(sizeof(char) * buffer_length);
	memset(buffer, '\0', sizeof(char) * buffer_length);
	memcpy(buffer, nickname, strnlen(nickname, 15));
	memcpy(buffer + 15, msg, msg_length);
	int sent_bytes = send_msg(socket, (uchar *)buffer, buffer_length, CHAT_FWD_MSG);
	free(buffer);
	return sent_bytes;
}

int send_user_list(int socket, linked_list_t *users)
{
	char *msg = (char *)malloc(sizeof(char) * 16 * users->count);
	memset(msg, '\0', sizeof(char) * 16 * users->count);
	int offset = 0;
	for (node_t *i = users->head->next; i != users->tail; i = i->next){
		memcpy(msg +  offset, i->client->name, 16);
		offset += 16;
	}
	int bytes_sent = 1;
	//TODO SEND MSG

	free(msg);
	return bytes_sent;
}

int send_fwd_client_left(int socket, char *nickname, char *quit_message)
{
	int message_length = strlen(quit_message) + 1 + 15;
	char *buffer = (char*)malloc(sizeof(char) * message_length);
	memset(buffer, '\0', message_length);
	sprintf(buffer, "%s%s", nickname, quit_message);
	int sent_bytes = 1;// send_msg(socket, (uchar *)buffer, CLIENT_LEFT_MSG);
	free(buffer);
	return sent_bytes;
}

int send_error(int socket, char *msg)
{
	return 1; //send_msg(socket, (uchar *)msg, ERROR_MSG);
}

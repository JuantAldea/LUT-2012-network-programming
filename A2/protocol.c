#include "protocol.h"

int send_msg(int socket, uchar *msg, uint8_t type)
{
	uint16_t msg_size = strlen((char *)msg) + 1;// add \0 => + 1
	uint16_t wrapped_size = msg_size + PROTOCOL_HEADER_SIZE;

	uchar * wrapped_msg = (uchar *) malloc(sizeof(uchar) * wrapped_size);
	memset(wrapped_msg, 0, wrapped_size);
	*(uint16_t*) wrapped_msg = htons(wrapped_size);
	wrapped_msg[sizeof(uint16_t)] = type;
	memcpy(&wrapped_msg[sizeof(uint8_t) + sizeof(uint16_t)], msg, msg_size*sizeof(uchar));

	int bytes_to_send = wrapped_size * sizeof(uchar);
	int total_sent_bytes = 0;

	while (total_sent_bytes < bytes_to_send){
		total_sent_bytes += send(socket, &(wrapped_msg[total_sent_bytes]), bytes_to_send - total_sent_bytes, 0);
	}
	free(wrapped_msg);
	return total_sent_bytes;
}

// int peek_msg(int socket)
// {
// 	uint16_t msg_size = -1;
// 	ioctl (socket, FIONREAD, &buffered_bytes);
// 	if (bytes_availables < 2 && bytes_availables != 0){b
// 		return -1;
// 	}
// 	int recv_bytes = recv(socket, &msg_size, sizeof(uint16_t), MSG_PEEK);
// 	if (recv_bytes == 0){
// 		return 0; //disconnect
// 	}

// 	if (msg_size > buffered_bytes){
// 		return -1;//not the full msg
// 	}
// 	return 1;//full msg received
// }

int recv_msg(int socket, uchar **msg, uint8_t *type)
{
	uint16_t msg_size = -1;
	int recv_bytes = recv(socket, &msg_size, sizeof(uint16_t), MSG_PEEK);

	if (recv_bytes == 0){
		return 0;
	}else if (recv_bytes < 0){
		return -1;
	}else if (recv_bytes < 2){
		perror("Discarting partial message\n");
	}

	recv_bytes = recv(socket, &msg_size, sizeof(uint16_t), 0);
	msg_size = ntohs(msg_size);
	msg_size -= PROTOCOL_HEADER_SIZE;
	//msg_size += 1;

	uint8_t received_type = -1;
	recv_bytes = recv(socket, &received_type, sizeof(uint8_t), MSG_PEEK);
	if (recv_bytes == 0){
		return 0;
	}else if (recv_bytes < 0){
		return -1;
	}
	recv_bytes = recv(socket, &received_type, sizeof(uint8_t), 0);
	*msg = (uchar *) malloc((msg_size + 1)* sizeof(uchar));//add new \0, just in case
	msg[msg_size] = '\0';

	recv_bytes = recv(socket, *msg, msg_size * sizeof(uchar), MSG_PEEK);

	if (recv_bytes == 0){
		free(*msg);
		return 0;
	}else if(recv_bytes < 0){
		free(*msg);
		return -1;
	}else if(recv_bytes < msg_size){
		free(*msg);
		return -2;
	}
	recv_bytes = recv(socket, *msg, msg_size * sizeof(uchar), 0);
	*type = received_type;
	return recv_bytes; //payload size
}

int send_login(int socket, uchar *username)
{
	return send_msg(socket, username, CONNECT_MSG);
}

int send_disconnect(int socket, double delay)
{
	uchar *buffer = (uchar*)malloc(100 * sizeof(uchar));
	sprintf((char*)buffer, "%.3f", delay);
	int bytes_sent = send_msg(socket, buffer, QUIT_MSG);
	free(buffer);
	return bytes_sent;
}


int send_error(int socket, uchar *msg)
{
	return send_msg(socket, msg, ERROR_MSG);
}

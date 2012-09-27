#include "recv_buffer.h"

void recv_buffer_reset(recv_buffer_t *buffer)
{
	if(buffer != NULL){
		buffer->message_length = -1;
		buffer->message_type = -1;
		buffer->received_bytes = 0;
		if(buffer->buffer != NULL){
			free(buffer->buffer);
			buffer->buffer = NULL;
		}
	}
}

void recv_buffer_free(recv_buffer_t *buffer)
{
	if (buffer != NULL){
		if (buffer->buffer != NULL){
			free(buffer->buffer);
		}
		free(buffer);
	}
}
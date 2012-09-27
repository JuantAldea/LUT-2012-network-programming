/*
###############################################
#		CT30A5001 - Network Programming		  #
#		Assignment2: TCP multiuser chat		  #
# 	Juan Antonio Aldea Armenteros (0404450)   #
# 		juan.aldea.armenteros@lut.fi		  #
#				recv_buffer.c				  #
###############################################
*/

#include "recv_buffer.h"

//clean and init the data of the buffer
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

//free the buffer memory
void recv_buffer_free(recv_buffer_t *buffer)
{
	if (buffer != NULL){
		if (buffer->buffer != NULL){
			free(buffer->buffer);
		}
		free(buffer);
	}
}
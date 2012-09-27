/*
###############################################
#		CT30A5001 - Network Programming		  #
#		Assignment2: TCP multiuser chat		  #
# 	Juan Antonio Aldea Armenteros (0404450)   #
# 		juan.aldea.armenteros@lut.fi		  #
#				recv_buffer.h				  #
###############################################
*/

#ifndef __RECV_BUFFER_T__
#define __RECV_BUFFER_T__

#include <stdlib.h>

#define uchar unsigned char

typedef struct recv_buffer_s recv_buffer_t;
struct recv_buffer_s{
	uchar *buffer;
	int32_t message_length;
	int32_t received_bytes;
	int16_t message_type;
};

void recv_buffer_reset(recv_buffer_t *buffer);
void recv_buffer_free(recv_buffer_t *buffer);
#endif

/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              protocol.h               #
####################################################
*/

#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "sctp_utils.h"

#define JOIN_MSG   0
#define OK_MSG     1
#define READY_MSG  2
#define START_MSG  3
#define TURN_MSG   4
#define COLUMN_MSG 5
#define AREA_MSG   6
#define WINNER_MSG 7
#define ERROR_MSG  1000

#define ERROR_GAME_RUNNING  0
#define ERROR_GAME_FULL     1
#define ERROR_NOT_YOUR_TURN 2
#define ERROR_COLUMN_FULL   3

#define PROTOCOL_PAYLOAD_IDENTIFER 2155905160

int send_msg(int sctp_sock, char *data_to_send, size_t byte_count, SA *server_addr, socklen_t server_addr_len);

int send_ok(int sctp_sock, int8_t player_count, int8_t player_id, SA *server_addr, socklen_t server_addr_len);

int send_join(int sctp_sock, SA *server_addr, socklen_t server_addr_len);

int send_ready(int sctp_sock, SA *server_addr, socklen_t server_addr_len);

int send_start(int sctp_sock, int8_t count_column, int8_t count_row, SA *addr, socklen_t addr_len);

int send_turn(int sctp_sock, SA *server_addr, socklen_t server_addr_len);

int send_column(int sctp_sock, int8_t column, SA *server_addr, socklen_t server_addr_len);

int send_winner(int sctp_sock, int8_t winner_id, SA *server_addr, socklen_t server_addr_len);

int send_error(int sctp_sock, int8_t error_code, char error_msg[], SA *server_addr, socklen_t server_addr_len);

int send_area(int sctp_sock, char *area, int8_t rows, int8_t columns, SA *server_addr, socklen_t server_addr_len);

int drop_connection(int sctp_sock, SA *server_addr, socklen_t server_addr_len);

char *pack_area(char *grid, int8_t rows, int8_t columns);
char *unpack_area(char *vector, int8_t rows, int8_t columns);

#endif
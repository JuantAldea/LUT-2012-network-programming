/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              protocol.c               #
####################################################
*/

#include "protocol.h"

int send_msg(int sctp_sock, char *data_to_send_msg, size_t byte_count, SA *server_addr, socklen_t server_addr_len)
{
    return sctp_sendmsg(
                    sctp_sock,          // socket file descriptor
                    data_to_send_msg,   // Data to send_msg
                    byte_count,         // Amount of data to send_msg in bytes
                    server_addr,        // address of receiver
                    server_addr_len,    // length of address structure
                    htonl(PROTOCOL_PAYLOAD_IDENTIFER),
                    0,         // flags
                    0,         // stream id
                    0,         // time to live, 0 = infinite
                    0);
}

int send_join(int sctp_sock, SA *server_addr, socklen_t server_addr_len)
{
    int16_t message = htons(JOIN_MSG);
    return send_msg(sctp_sock, (char*)&message, sizeof(int16_t), server_addr, server_addr_len);
}

int send_ok(int sctp_sock, int8_t player_count, int8_t player_id, SA *server_addr, socklen_t server_addr_len)
{
    char *message = (char *)malloc(sizeof(int16_t) + 2 * sizeof(int8_t));
    *(int16_t*)message = htons(OK_MSG);
    *(int8_t*)((int16_t*)message + 1) = player_count;
    *((int8_t*)((int16_t*)message + 1) + 1) = player_id;
    int sent_bytes = send_msg(sctp_sock, message, sizeof(int16_t) + 2 * sizeof(int8_t), server_addr, server_addr_len);
    free(message);
    return sent_bytes;
}

int send_ready(int sctp_sock, SA *server_addr, socklen_t server_addr_len)
{
    int16_t message = htons(READY_MSG);
    return send_msg(sctp_sock, (char*)&message, sizeof(int16_t), server_addr, server_addr_len);
}

int send_start(int sctp_sock, int8_t count_column, int8_t count_row, SA *addr, socklen_t addr_len)
{
    char *message = (char *)malloc(sizeof(int16_t) + 2 * sizeof(int8_t));
    *(int16_t*)message = htons(START_MSG);
    *(int8_t*)((int16_t*)message + 1) = count_column;
    *((int8_t*)((int16_t*)message + 1) + 1) = count_row;
    int sent_bytes = send_msg(sctp_sock, message, sizeof(int16_t) + 2 * sizeof(int8_t), addr, addr_len);
    free(message);
    return sent_bytes;
}

int send_turn(int sctp_sock, SA *server_addr, socklen_t server_addr_len)
{
    int16_t message = htons(TURN_MSG);
    return send_msg(sctp_sock, (char*)&message, sizeof(int16_t), server_addr, server_addr_len);
}

int send_column(int sctp_sock, int8_t column, SA *server_addr, socklen_t server_addr_len)
{
    char *message = (char *)malloc(sizeof(int16_t) + sizeof(int8_t));
    *(int16_t*)message = htons(COLUMN_MSG);
    *(int8_t*)((int16_t*)message + 1) = column;
    int sent_bytes = send_msg(sctp_sock, message, sizeof(int16_t) + sizeof(int8_t), server_addr, server_addr_len);
    free(message);
    return sent_bytes;
}

int send_winner(int sctp_sock, int8_t winner_id, SA *server_addr, socklen_t server_addr_len)
{
    char *message = (char *)malloc(sizeof(int16_t) + sizeof(int8_t));
    *(int16_t*)message = htons(WINNER_MSG);
    *(int8_t*)((int16_t*)message + 1) = winner_id;
    int sent_bytes = send_msg(sctp_sock, message, sizeof(int16_t) + sizeof(int8_t), server_addr, server_addr_len);
    free(message);
    return sent_bytes;
}

int send_error(int sctp_sock, int8_t error_code, char error_msg[], SA *server_addr, socklen_t server_addr_len)
{
    int msg_len = strlen(error_msg) * sizeof(char) + sizeof(int8_t) + sizeof(int16_t) + 1;
    char *message = (char *)malloc(msg_len);
    memset(message, '\0', msg_len);
    *(int16_t*)message = htons(ERROR_MSG);
    *(uint8_t*)((int16_t*)message + 1) = error_code;
    strcpy((char*)(((int8_t*)((int16_t*)message + 1)) + 1), error_msg);
    printf("error: %d\n", ERROR_MSG);
    int sent_bytes = send_msg(sctp_sock, message, msg_len, server_addr, server_addr_len);
    free(message);
    return sent_bytes;
}

int drop_connection(int sctp_sock, SA *server_addr, socklen_t server_addr_len)
{
    char buffer = 0;
    return sctp_sendmsg(
                    sctp_sock,          // socket file descriptor
                    &buffer,   // Data to send_msg
                    0,         // Amount of data to send_msg in bytes
                    server_addr,        // address of receiver
                    server_addr_len,    // length of address structure
                    PROTOCOL_PAYLOAD_IDENTIFER,
                    SCTP_EOF,         // flags
                    0,         // stream id
                    0,         // time to live, 0 = infinite
                    0);
}

char* pack_area(char *grid, int8_t rows, int8_t columns)
{
    char *vector = (char*)malloc((rows * columns + rows) * sizeof(char));
    memset(vector, 0, (rows * columns + rows) * sizeof(char));
    int i = 0;
    for (i = 0; i < rows;i++){
        memcpy(vector + i * (columns + 1), grid + (rows - 1 - i) * columns, columns);
    }
    return vector;
}

char* unpack_area(char *vector, int8_t rows, int8_t columns)
{
    char *grid = (char*)malloc(rows * columns * sizeof(char));
    for (int i = 0; i< rows; i++){
        memcpy(grid + columns * (rows - 1 - i), vector + i*(columns + 1), columns);
    }
    return grid;
}

int send_area(int sctp_sock, char *area, int8_t rows, int8_t columns, SA *server_addr, socklen_t server_addr_len)
{
    char *packed_area = pack_area(area, rows, columns);
    int packed_area_size = (rows * columns + rows) * sizeof(char);
    char *buffer = (char*)malloc(sizeof(int16_t) + packed_area_size);
    *(int16_t*)buffer = htons(AREA_MSG);
    memcpy((int16_t*)buffer + 1, packed_area, packed_area_size);
    free (packed_area);
    int bytes = send_msg(sctp_sock, buffer, sizeof(int16_t) + packed_area_size, server_addr, server_addr_len);
    free(buffer);
    return bytes;
}

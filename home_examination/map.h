#ifndef __MAP_H__
#define __MAP_H__

#include "system_headers.h"
#include "md5.h"
#include <inttypes.h>

#define SERVER_MAP_FOLDER "serverdata"
#define CLIENT_MAP_FOLDER "clientdata"
#define MAP_FILE_EXTENSION ".map"
#define MAX_MSG_SIZE 1024
#define BLOCKSMAX 100
#define BLOCKSIZE 2
#define MAXPLAYERS 6
typedef struct map_s map_t;
struct map_s{
    char hash[33];
    char map_id[9];
    uint8_t rows;
    uint8_t colums;
    uint8_t frag_limit;
    uint8_t max_players;
    uint8_t number_of_blocks;
    uint8_t starting_positions[MAXPLAYERS][BLOCKSIZE];
    uint8_t block_positions[BLOCKSMAX][BLOCKSIZE];
};

int read_map(char *path, map_t *map);
int recv_map(int socket, char *map_id);
int send_map(int socket, char *map_id);
void print_map(map_t map);

#endif

/*
map_id <int><newline>
rows   <int><newline>
colums <int><newline>
number of blocks <int><newline>
x y {<int><space><int><newline>}*
[...]
max players <int><newline>
x y {<int><space><int><newline>}*
[...]
fraglimit <int>{<newline>, 0, 1}
*/

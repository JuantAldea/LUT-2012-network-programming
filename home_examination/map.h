#ifndef __MAP_H__
#define __MAP_H__

#include "system_headers.h"
#include "md5.h"

#define SERVER_MAP_FOLDER "serverdata"
#define CLIENT_MAP_FOLDER "clientdata"
#define MAP_FILE_EXTENSION ".map"
#define MAX_MSG_SIZE 1024

typedef struct map_s map_t;
struct map_s{
    char hash[33];
    char map_id[9];
    int rows;
    int colums;
    int frag_limit;
    int max_players;
    int number_of_blocks;
    int **starting_positions; /* starting_positions[max_players][2] */
    int **block_positions; /* starting_positions[number_of_blocks][2] */
};

void free_map(map_t *map);
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

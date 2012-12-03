#include "map.h"

int read_map(char *path, map_t *map)
{
    FILE *map_file = fopen(path, "r");
    memset(map, 0, sizeof(map_t));
    md5_from_file(path, map->hash);
    size_t size = 255;
    char *buffer = malloc(size);
    //map id
    getline(&buffer, &size, map_file);
    sscanf(buffer,"%s", map->map_id);
    //rows
    getline(&buffer, &size, map_file);
    sscanf(buffer,"%"SCNu8, &map->rows);
    // //colums
    getline(&buffer, &size, map_file);
    sscanf(buffer,"%"SCNu8, &map->colums);
    //number of blocks
    getline(&buffer, &size, map_file);
    sscanf(buffer,"%"SCNu8, &map->number_of_blocks);
    //blocks
    for(int i = 0; i < map->number_of_blocks; i++){
        getline(&buffer, &size, map_file);
        sscanf(buffer,"%"SCNu8" %"SCNu8, &map->block_positions[i][0], &map->block_positions[i][1]);
    }
    //max players
    getline(&buffer, &size, map_file);
    sscanf(buffer,"%"SCNu8, &map->max_players);
    //starting positions
    for(int i = 0; i < map->max_players; i++){
        getline(&buffer, &size, map_file);
        sscanf(buffer,"%"SCNu8" %"SCNu8, &map->starting_positions[i][0], &map->starting_positions[i][1]);
    }
    //frag limit
    getline(&buffer, &size, map_file);
    sscanf(buffer,"%"SCNu8, &map->frag_limit);
    fclose(map_file);
    free(buffer);
    buffer = NULL;
    return 0;
}

int send_map(int socket, char *map_id)
{
    int error = 0;

    uchar buffer[MAX_MSG_SIZE];
    char *map_path = malloc(strlen(map_id) + strlen(SERVER_MAP_FOLDER) + strlen(MAP_FILE_EXTENSION) + 2);
    sprintf(map_path, "%s/%s%s", SERVER_MAP_FOLDER, map_id, MAP_FILE_EXTENSION);
    FILE *source = fopen(map_path, "rb");
    printf("Send map %s\n", map_path);
    if (source == NULL){
        printf("ERROR opening the file %s\n", strerror(errno));
        return -1;
    }

    sprintf(map_path, "%s/%s%s", SERVER_MAP_FOLDER, map_id, MAP_FILE_EXTENSION);
    free(map_path);

    //send map_id length
    int8_t map_id_length = (int8_t)strlen(map_id);
    send(socket, &map_id_length, sizeof(uint8_t), 0);
    //send map_id
    send(socket, map_id, strlen(map_id), 0);
    //send file
    int readed_bytes = 0;
    int total_sent_bytes = 0;
    while((fread(buffer, sizeof(char), MAX_MSG_SIZE, source)) > 0 && !error){
        int sent_bytes = 0;
        while (sent_bytes < readed_bytes && !error){
            int last_sent_bytes = send(socket, &(buffer[sent_bytes]), readed_bytes - sent_bytes, 0);
            if (last_sent_bytes < 0){
                printf("ERROR sending file %s\n", strerror(errno));
                error = 1;
            }
            sent_bytes += last_sent_bytes;
            total_sent_bytes += sent_bytes;
        }
    }
    fclose(source);
    return error == 0 ? total_sent_bytes : -1;
}

int recv_map(int socket, char *map_id)
{
    uchar buffer[MAX_MSG_SIZE];
    uint8_t map_id_length;
    int received_bytes = recv(socket, &map_id_length, sizeof(uint8_t), 0);
    //recv id
    received_bytes = recv(socket, map_id, map_id_length, 0);
    char *path = malloc(sizeof(char) * (8 * strlen(CLIENT_MAP_FOLDER) + strlen(MAP_FILE_EXTENSION) + 2));
    sprintf(path, "%s/%.*s%s", CLIENT_MAP_FOLDER, 8, map_id, MAP_FILE_EXTENSION);
    printf("%s\n", path);

    int destination = open(path, O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR);

    if (destination < 0){
        printf("ERROR opening the file %s\n", strerror(errno));
        return 0;
    }
    //recv file
    int total_bytes_received = 0;
    received_bytes = 0;
    while((received_bytes = recv(socket, buffer, MAX_MSG_SIZE, 0)) > 0){
        total_bytes_received += received_bytes;
        if (write(destination, buffer, received_bytes) <= 0){
            printf("ERROR writing the file %s\n", strerror(errno));
            return -1;
        }
    }

    close(destination);
    free(path);
    return total_bytes_received;
}

void print_map(map_t map)
{
    printf("Hash:    %.*s\n", 32, map.hash);
    printf("ID:      %s\n", map.map_id);
    printf("Rows:    %"SCNu8"\n", map.rows);
    printf("Cols:    %"SCNu8"\n", map.colums);
    printf("Frags:   %"SCNu8"\n", map.frag_limit);
    printf("Players: %"SCNu8"\n", map.max_players);
    for (int i=0; i < map.max_players; i++){
        printf("\tPlayer %"SCNu8": %"SCNu8" %"SCNu8"\n", i+1, map.starting_positions[i][0], map.starting_positions[i][1]);
    }
    printf("Blocks %d\n", map.number_of_blocks);
    for (int i=0; i < map.number_of_blocks; i++){
        printf("\tBlock %"SCNu8": %"SCNu8" %"SCNu8"\n", i+1, map.block_positions[i][0], map.block_positions[i][1]);
    }
}
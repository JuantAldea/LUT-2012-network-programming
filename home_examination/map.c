#include "map.h"

void free_map(map_t *map)
{
    for (int i = 0; i < map->number_of_blocks; i++){
        free(map->block_positions[i]);
    }
    free(map->block_positions);

    for (int i = 0; i < map->max_players; i++){
        free(map->starting_positions[i]);
    }
    free(map->starting_positions);
}

int read_map(char *path, map_t *map)
{
    char *buffer;
    FILE *map_file = fopen(path, "r");
    md5_from_file(path, map->hash);

    int readed_bytes;
    size_t size;
    //map id
    readed_bytes = getline(&buffer, &size, map_file);
    sscanf(buffer,"%d", &map->map_id);
    free(buffer);
    buffer = NULL;
    //rows
    readed_bytes = getline(&buffer, &size, map_file);
    sscanf(buffer,"%d", &map->rows);
    free(buffer);
    buffer = NULL;
    //colums
    readed_bytes = getline(&buffer, &size, map_file);
    sscanf(buffer,"%d", &map->colums);
    free(buffer);
    buffer = NULL;
    //number of blocks
    readed_bytes = getline(&buffer, &size, map_file);
    sscanf(buffer,"%d", &map->number_of_blocks);
    free(buffer);
    buffer = NULL;
    //blocks
    map->block_positions = malloc(sizeof(int*) * map->number_of_blocks);
    for(int i = 0; i < map->number_of_blocks; i++){
        map->block_positions[i] = malloc(sizeof(int*) * 2);
        readed_bytes = getline(&buffer, &size, map_file);
        sscanf(buffer,"%d %d", &map->block_positions[i][0], &map->block_positions[i][1]);
        free(buffer);
        buffer = NULL;
    }
    //max players
    readed_bytes = getline(&buffer, &size, map_file);
    sscanf(buffer,"%d", &map->max_players);
    free(buffer);
    buffer = NULL;
    //starting positions
    map->starting_positions = malloc(sizeof(int*) * map->max_players);
    for(int i = 0; i < map->max_players; i++){
        map->starting_positions[i] = malloc(sizeof(int*) * 2);
        readed_bytes = getline(&buffer, &size, map_file);
        sscanf(buffer,"%d %d", &map->starting_positions[i][0], &map->starting_positions[i][1]);
        free(buffer);
        buffer = NULL;
    }
    //frag limit
    readed_bytes = getline(&buffer, &size, map_file);
    sscanf(buffer,"%d", &map->frag_limit);
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
    printf("%s\n", map_path);
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
    while((readed_bytes = fread(buffer, sizeof(char), MAX_MSG_SIZE, source)) > 0 && !error){
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

int recv_map(int socket, char **map_id)
{
    uchar buffer[MAX_MSG_SIZE];
    uint8_t map_id_length;
    //recv size of the id
    printf("%d\n", map_id_length);
    int received_bytes = recv(socket, &map_id_length, sizeof(uint8_t), 0);

    *map_id = malloc(sizeof(char) * (map_id_length + 1));
    //recv id
    received_bytes = recv(socket, *map_id, map_id_length, 0);
    char *path = malloc(sizeof(char) * (strlen(CLIENT_MAP_FOLDER) + strlen(MAP_FILE_EXTENSION) + 2));
    sprintf(path, "%s/%s%s", CLIENT_MAP_FOLDER, *map_id, MAP_FILE_EXTENSION);
    printf("%s\n", path);

    int destination = open(path, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);

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

void print_map(map_t *map)
{
    printf("ID:      %d\n", map->map_id);
    printf("Rows:    %d\n", map->rows);
    printf("Cols:    %d\n", map->colums);
    printf("Frags:   %d\n", map->frag_limit);
    printf("Players: %d\n", map->max_players);
    for (int i=0; i < map->max_players; i++){
        printf("\tPlayer %d: %d %d\n", i+1, map->starting_positions[i][0], map->starting_positions[i][1]);
    }
    printf("Blocks %d\n", map->number_of_blocks);
    for (int i=0; i < map->number_of_blocks; i++){
        printf("\tBlock %d: %d %d\n", i+1, map->block_positions[i][0], map->block_positions[i][1]);
    }
    printf("Hash %32s\n", map->hash);
}
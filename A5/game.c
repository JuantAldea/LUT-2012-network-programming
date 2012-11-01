/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              game.c                              #
####################################################
*/
#include "game.h"

void movement_order(linked_list_t *player_list)
{
    int jugadores = 4;
    int order[4];
    int8_t assigned_turns[jugadores];
    memset(assigned_turns, 0, sizeof(int8_t) * jugadores);
    srand(time(NULL));
    int current_player = 0;
    while(current_player < jugadores){
        int current_turn = rand() % jugadores;
        if (assigned_turns[current_turn] == 0){
            assigned_turns[current_turn] = 1;
            order[current_turn] = current_player;
            current_player++;
        }
    }
    for (int i = 0; i < jugadores; i++){
        printf("%d ", order[i]);
    }
    printf("\n");
}


int insert_disc(char *grid, int rows, int columns, int player, int column)
{
    if (0 <= column && column < columns){
        for (int i = rows - 1; i >= 0; i--){
            if (grid[i * columns + column] == 'x'){
                grid[i * columns + column] = player + '0';
                return 1;
            }
        }
    }
    return 0;
}

int check_winner(char *grid, int rows, int columns)
{
    int discs = 0;
    for (int i = 0; i < rows; i++){
        for (int j = 0; j < columns; j++){
            char player = grid[i * columns + j];
            if (player != 'x'){
                discs++;
                // Check whether it is a winner horizontally
                int counter = 0;
                for (int k = j; k < j + 4; k++) {
                    if (k < columns) {
                        if (grid[i * columns + k] == player) {
                            counter++;
                        }
                    }
                }
                if (counter == 4) {
                    return player - '0';
                }

                // Check whether it is a winner vertically
                counter = 0;
                for (int k = i; k < i + 4; k++) {
                    if (k < rows) {
                        if (grid[k * columns + j] == player) {
                            counter++;
                        }
                    }
                }
                if (counter == 4) {
                    return player - '0';
                }

                // Check whether it is a winner diagonally (down-right)
                counter = 0;
                for (int k = 0; k < 4; k++) {
                    if (i + k < rows && j + k < columns) {
                        if (grid[(i + k) * columns + (j + k)] == player) {
                            counter++;
                        }
                    }
                }
                if (counter == 4) {
                    return player - '0';
                }

                // Check whether it is a winner diagonally (up-right)
                counter = 0;
                for (int k = 0; k < 4; k++) {
                    if (i - k >= 0 && j + k < columns) {
                        if (grid[(i - k) * columns + (j + k)] == player) {
                            counter++;
                        }
                    }
                }
                if (counter == 4) {
                    return player - '0';
                }
            }
        }
    }
    return (discs == rows * columns) ? 0 : -1;
}

void print_grid(char *grid, int rows, int columns)
{
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            switch (grid[i * columns + j]) {
                case '1': printf("\033[44m\033[1m\033[31m o \033[0m"); break;
                case '2': printf("\033[44m\033[1m\033[33m o \033[0m"); break;
                case '3': printf("\033[44m\033[1m\033[32m o \033[0m"); break;
                case '4': printf("\033[44m\033[1m\033[36m o \033[0m"); break;
                default : printf("\033[44m\033[1m\033[30m   \033[0m"); break;
            }
        }
        printf("\n");
    }
}
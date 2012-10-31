
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

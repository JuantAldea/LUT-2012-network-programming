/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#              game.h               #
####################################################
*/
#ifndef __GAME_H__
#define __GAME_H__

#include "addresses.h"
#include "linked_list.h"
#include <time.h>

void movement_order(linked_list_t *players);

int insert_disc(char *grid, int rows, int columns, int player, int column);

int check_winner(char *grid, int rows, int columns);

void print_grid(char *grid, int rows, int columns);
#endif
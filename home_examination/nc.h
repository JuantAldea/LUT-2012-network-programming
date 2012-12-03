/*
###############################################
#        CT30A5001 - Network Programming      #
#               Home Examination              #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################
*/

#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <signal.h>
#include <stdint.h>
#include <string.h>
#include "system_headers.h"
#include "game_protocol.h"
#include "chat_protocol.h"
#include "udp.h"
#include "tcp.h"
// Predefined widht and height - only for demonstration
#define WIDTH 20
#define HEIGHT 15
// Horizontal line - with adjustable width you should make this dynamic
#define LINEW WIDTH*4+1

// Chat message buffer length
#define BUFFER 128

// Visible log size
#define LOGSIZE 5

// Player health
#define HEALTH 5

// Wall color and hit colors
#define WALL_COLOR 5
#define PLAYER_HIT_COLOR 6

// Player identities - work as ncurses  colors
#define PLAYER1 1
#define PLAYER2 2
#define PLAYER3 3
#define PLAYER4 4
#define PLAYER5 5
#define PLAYER6 6

// For blocks in the area, each coordinate consists of 2 numbers, 100 blocks max
#define BLOCKSMAX 100
#define BLOCKSIZE 2

// Game struct
typedef struct t_gamearea {
  char* array; // Game area
  uint8_t width; // width of game area
  uint8_t height; // height of game area
  uint8_t blocks[BLOCKSMAX][BLOCKSIZE]; // Walls in the area, i.e. blocks
  int blockcount; // amount of walls in the game area
} gamearea;

typedef struct t_player {
  uint8_t id; // Id and color
  uint8_t posx; // x coordinate
  uint8_t posy; // y coordinate
  uint8_t health; // player health
  uint8_t hitwall; // did player hit wall on last update
  uint8_t hitrepeat; // amount of repeated hits to walls
} player;

//New game area
// Params: width,height,blockcount, blocks
gamearea* new_gamearea(int,int,int, uint8_t blocks[][BLOCKSIZE]);
void free_gamearea(gamearea*);

// New player
// Params: id, startx, starty, health
player* new_player(uint8_t, uint8_t, uint8_t, uint8_t);
void free_player(player* p);

// is there a wall in coordinates
// Params:  gamearea, x, y
int is_position_a_wall(gamearea*, int, int);

// Draw the grid
//void ui_draw_grid(gamearea*, player*);
void ui_draw_grid(gamearea* game, uint8_t playerid, player *players[]);

// Draw ending, death = 1, player died, 0 = quit
void ui_draw_end(int death);

// Initialize the grid horizontal line
// Params:  widht = game area width!
void prepare_horizontal_line(int width);
void free_horizontal_line();

// Logging, add & clear
void add_log(char*, int);
void clear_log();

void sighandler(int);

double time_diff(struct timeval *after, struct timeval *before);


#include "nc.h"

// Textinput from ui
char textbuffer[BUFFER] = {0};
int textinput = 0;

// Chatlog
char chatlog[LOGSIZE][BUFFER];
int logposition = 0;
int logentries = 0;

// Some status variables
int spectator_mode = 0;
int status = 0;
uint8_t deaths = 0;
uint8_t frags = 0;
// Horizontal line for grid
char* line = NULL;
double latency = 0;
gamearea* new_gamearea(int width, int height, int blockcount, uint8_t blocks[][BLOCKSIZE]) {
  // Allocate game struct
  gamearea *game = (gamearea*)malloc(sizeof(gamearea));

  // Set the game area
  game->array = (char*)malloc(width*height);
  game->width = width;
  game->height = height;
  memset(game->array,0,width*height);

  // Set the blocks
  if(blockcount > BLOCKSMAX) blockcount = BLOCKSMAX;
  memcpy(game->blocks,blocks,sizeof(uint8_t)*blockcount*BLOCKSIZE);
  game->blockcount = blockcount;

  return game;
}

void free_gamearea(gamearea* game) {
  if(game) {
    free(game->array);
    free(game);
  }
}

// New player
player* new_player(uint8_t id, uint8_t startx, uint8_t starty, uint8_t health) {
  player *p = (player*)malloc(sizeof(p));
  p->id = id;
  p->posx = startx;
  p->posy = starty;
  p->health = health;
  p->hitwall = 0;
  p->hitrepeat = 0;
  return p;
}

void free_player(player* p) {
  if(p) free(p);
}

int is_position_a_wall(gamearea *game, int posx, int posy)
{
  for(int i = 0; i < game->blockcount; i++){
    if(game->blocks[i][0] == posx && game->blocks[i][1] == posy){
      return 1;
    }
  }
  // Right wall
  if(posx >= game->width) return 1;
  // Left wall
  if(posx < 0) return 1;
  // Bottom wall
  if(posy >= game->height) return 1;
  // Top wall
  if(posy < 0) return 1;

  return 0;
}

void ui_draw_grid(gamearea* game, uint8_t playerid, player *players[])
{
  int x = 0, y = 0, index = 0, was_wall = 0;
  char pchar = ' ';
  player *pl = NULL;
  if (playerid < 7){
    pl = players[playerid - 1];
  }
  clear(); // clear screen
  if (pl != NULL){
    if(pl->hitwall){
      pl->hitrepeat++;
    }else{
      pl->hitrepeat = 0;
    }
    printw("Player ");
    attron(COLOR_PAIR(pl->id));
    printw("%d",pl->id);
    attroff(COLOR_PAIR(pl->id));
    printw(" at x = %d, y = %d | Score: %d/%d | Latency: %fms\n", pl->posx, pl->posy, frags, deaths, latency);
    attron(COLOR_PAIR(pl->id));
    printw("LIFE[%d]",pl->health);
    attroff(COLOR_PAIR(pl->id));
    printw("\n%s\n", pl->hitwall ? "OUCH! HIT A WALL (LOST 1 HEALTH)" : "TRAVELLING...");
  }else{
    attron(COLOR_PAIR(COLOR_WHITE));
    printw("\n\n\n");
    attroff(COLOR_PAIR(COLOR_WHITE));
  }
  printw("Game status: %s\n",status == 0 ? "not ready" : "ready" );
  printw("Connection status: %s\n",status == 0 ? "disconnected" : "connected" );
  printw("%s\n",line);

  // Draw grid
  for(index = 0; index < game->width * game->height; index++,x++) {
    // Start a new line?
    if(index % game->width == 0) {
      x = 0; // reset the x coordinate
      // The first index is always 0, skip it
      if(index > 0) {
        printw("|\n%s\n",line);
        y++;
      }
    }
    printw("|");
    pchar = ' '; //empty mark
    // The player
    for (int i = 0; i < 6; i++){
      if (players[i] != NULL && players[i]->posx == x && players[i]->posy == y && players[i]->health > 0){
        if(players[i]->hitwall){
          attron(COLOR_PAIR(PLAYER_HIT_COLOR)); // Did player hit a wall
          pchar = 'o'; // Player mark
          //attroff(COLOR_PAIR(PLAYER_HIT_COLOR)); // Did player hit a wall
        }else{
          attron(COLOR_PAIR(players[i]->id)); // Normal color
          pchar = 'o'; // Player mark
          //attroff(COLOR_PAIR(players[i]->id)); // Normal color
        }
      }
    }
    // if(pl != NULL && x == pl->posx && y == pl->posy) {
    //   if(pl->hitwall) attron(COLOR_PAIR(PLAYER_HIT_COLOR)); // Did player hit a wall
    //   else attron(COLOR_PAIR(pl->id)); // Normal color
    //   pchar = 'o'; // Player mark
    // }else{
    //   pchar = ' '; // Otherwise empty mark
    // }

    // Is it a wall
    if(is_position_a_wall(game,x,y) == 1) {
      attron(COLOR_PAIR(WALL_COLOR)); // Paint it white
      was_wall = 1;
    }

    // Put the mark on the grid
    printw(" %c ",pchar);

    for (int i = 0; i < 6; i++){
      if (players[i] != NULL && players[i]->posx == x && players[i]->posy == y && players[i]->health > 0){
        if(players[i]->hitwall){
          attroff(COLOR_PAIR(PLAYER_HIT_COLOR)); // Did player hit a wall
        }else{
          attroff(COLOR_PAIR(players[i]->id)); // Normal color
        }
      }
    }

    // If player hit a wall disable the color
    if (pl != NULL){
      if(pl->hitwall) attroff(COLOR_PAIR(PLAYER_HIT_COLOR));
      else attroff(COLOR_PAIR(pl->id));
    }

    // Wall painted, disable the color
    if(was_wall == 1) attroff(COLOR_PAIR(WALL_COLOR));
    was_wall = 0;
  }
  printw("|\n%s\n",line);
  if(pl != NULL && pl->hitrepeat > 1) printw("STOP HITTING THE WALL YOU'LL KILL YOURSELF");
  if(textinput==1) printw("\tTYPING: %s",textbuffer);
  printw("\n%s\n",line);
  printw("\nCHAT\n");
  for(int row = 0; row < LOGSIZE; row++) {
    if(strlen(&chatlog[row][0]) > 0){
      printw("\t%d: %s\n",row+logentries,&chatlog[row][0]);
    }
  }
  refresh();
}

void ui_draw_end(int death) {
  clear();
  attron(A_BOLD); // Bold text

  if(death) {
    // Blinking sequence
    for(int i = 0; i < 10; i++) {
      // On
      printw("YOU ARE DEAD!\n");
      refresh();
      usleep(200000);
      // Off
      clear();
      refresh();
      usleep(200000);
    }
  }

  clear();
  printw("Quitting.\n");
  attroff(A_BOLD);
  refresh();
}

void prepare_horizontal_line(int width) {
  line = (char*)malloc(sizeof(char)*(width*4+2));
  memset(line, 0, width*4+2);
  memset(line,'-',width*4+1);
}

void free_horizontal_line() {
  if(line) free(line);
}

void clear_log() {
  for(int i = 0; i < LOGSIZE; i++) {
    memset(&chatlog[i][0],0,BUFFER);
  }
}

void add_log(char* message, int msglen) {
  // Log is less than maximum
  if(logposition < LOGSIZE) {
    memset(&chatlog[logposition][0],0,BUFFER);
    memcpy(&chatlog[logposition][0],message,msglen); // Copy to the position
    logposition++; // Increase counter
  }
  // Log is full
  else {
    // Move the entries
    for(int i = 0; i < LOGSIZE-1; i++) memcpy(&chatlog[i][0],&chatlog[i+1][0],BUFFER);
    memset(&chatlog[LOGSIZE-1][0],0,BUFFER);
    memcpy(&chatlog[LOGSIZE-1][0],message,msglen); // Replace the last with the new one
    logentries++; // Increase counter
  }
}

void help(char *program){
    printf("Client %s -h <server address> -p <port>\n", program);
    return;
}

int is_number(char *str, int base, int *number)
{
    if (str != NULL){
        char *endptr;
        *number = strtol(str, &endptr, base);
        int return_value = (*str != '\0' && *endptr == '\0');
        return return_value;
    }
    return 0;
}

int client(char *address, int port)
{
  char game_port[20];
  char chat_port[20];
  char map_port[20];
  sprintf(game_port, "%d", port);
  sprintf(chat_port, "%d", port + 1);
  sprintf(map_port,  "%d", port + 2);
  int readc = 0, quit = 0, playerid = PLAYER1;
  int textpos = 0;
  int health;
  int ping_id;
  status = 0;
  // Game area
  gamearea* game = new_gamearea(WIDTH, HEIGHT, 0, NULL);

  //players
  player *players[6];
  for (int i = 0; i < 6; i++){
    players[i] = NULL;
  }


  initscr(); // Start ncurses
  //noecho(); // Disable echoing of terminal input
  cbreak(); // Individual keystrokes
  intrflush(stdscr, FALSE); // Prevent interrupt flush
  keypad(stdscr,TRUE); // Allow keypad usage

  start_color(); // Initialize colors

  // Color pairs init_pair(colorpair id,foreground color, background color)
  init_pair(PLAYER1,PLAYER1,COLOR_BLACK); // Player1 = COLOR_RED (1)
  init_pair(PLAYER2,PLAYER2,COLOR_BLACK); // Player2 = COLOR_GREEN (2)
  init_pair(PLAYER3,PLAYER3,COLOR_BLACK); // Player3 = COLOR_YELLOW (3)
  init_pair(PLAYER4,PLAYER4,COLOR_BLACK); // Player4 = COLOR_BLUE (4)
  init_pair(PLAYER5,PLAYER5,COLOR_BLACK); // Player4 = COLOR_BLUE (4)
  init_pair(PLAYER6,PLAYER6,COLOR_BLACK); // Player4 = COLOR_BLUE (4)

  init_pair(PLAYER_HIT_COLOR,COLOR_RED,COLOR_YELLOW);
  init_pair(WALL_COLOR,COLOR_WHITE,COLOR_WHITE);

  // Prepare everything
  clear_log();
  prepare_horizontal_line(WIDTH);

  ui_draw_grid(game, playerid, players);

  fd_set readfs;
  int rval = 0;
  struct sockaddr_storage server_sockaddr;
  socklen_t server_addrlen;
  int game_descriptor = prepare_client_UDP(address, game_port, (struct sockaddr *)&server_sockaddr, &server_addrlen);
  int chat_server_descriptor = -1;
  struct timeval timeout;
  struct timeval ping_time;
  gettimeofday(&ping_time, NULL);
  int number_of_lost_pings = 0;
  while(1) {
    FD_ZERO(&readfs);
    FD_SET(fileno(stdin), &readfs);
    FD_SET(game_descriptor, &readfs);
    if (chat_server_descriptor != -1){
      FD_SET(chat_server_descriptor, &readfs);
    }
    int max_fd = fileno(stdin) > game_descriptor ? fileno(stdin) : game_descriptor;
    max_fd = max_fd > chat_server_descriptor ? max_fd : chat_server_descriptor;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    // Block until we have something
    if((rval = select(max_fd + 1,&readfs, NULL, NULL, &timeout)) > 0) {
      // From user
      if(FD_ISSET(fileno(stdin),&readfs)) {
        readc = getch(); // Get each keypress
        switch(readc) {
          case KEY_LEFT:
          if(status){
              send_move(game_descriptor, KEY_LEFT, (struct sockaddr *)&server_sockaddr, server_addrlen);
            }
            //if(is_position_a_wall(game,pl1->posx-1,pl1->posy)) pl1->hitwall = 1;
            //else pl1->posx--;
            break;
          case KEY_RIGHT:
          if(status){
              send_move(game_descriptor, KEY_RIGHT, (struct sockaddr *)&server_sockaddr, server_addrlen);
            }
            //if(is_position_a_wall(game,pl1->posx+1,pl1->posy)) pl1->hitwall = 1;
            //else pl1->posx++;
            break;
          case KEY_UP:
            // if(is_position_a_wall(game,pl1->posx,pl1->posy-1)) pl1->hitwall = 1;
            // else pl1->posy--;
            if(status){
              send_move(game_descriptor, KEY_UP, (struct sockaddr *)&server_sockaddr, server_addrlen);
            }
            break;
          case KEY_DOWN:
            if(status){
              send_move(game_descriptor, KEY_DOWN, (struct sockaddr *)&server_sockaddr, server_addrlen);
            }
            //if(is_position_a_wall(game,pl1->posx,pl1->posy+1)) pl1->hitwall = 1;
            //else pl1->posy++;

            break;
          // Function keys, here F1 is reacted to
          case KEY_F(1):
            spectator_mode = 0;
            number_of_lost_pings = 0;
            if (!status){
              clear_log();
              logposition = 0;
              send_connect(game_descriptor, 1, (struct sockaddr *)&server_sockaddr, server_addrlen);
            }else{
              if(chat_server_descriptor > 0){
                close(chat_server_descriptor);
                chat_server_descriptor = -1;
                status = 0;
                for (int i = 0; i < 6; i++){
                  free_player(players[i]);
                  players[i] = NULL;
                }
              }
            }
            break;
          case KEY_F(2):
            spectator_mode = 1;
            number_of_lost_pings = 0;
            if (!status){
              clear_log();
              logposition = 0;
              send_connect(game_descriptor, 0, (struct sockaddr *)&server_sockaddr, server_addrlen);
            }else{
              if(chat_server_descriptor > 0){
                close(chat_server_descriptor);
                chat_server_descriptor = -1;
                status = 0;
                for (int i = 0; i < 6; i++){
                  free_player(players[i]);
                  players[i] = NULL;
                }
              }
            }
            break;
          case 27: // Escape key
            quit = 1;
            break;
          case '/':
            // User wants to write something
            memset(&textbuffer,0,BUFFER);
            textinput = 1;
            textpos = 0;
            break;
          // Erase text
          case KEY_BACKSPACE:
          case KEY_DC:
            textpos--;
            textbuffer[textpos] = '\0';
            break;
          // Push the line to log with enter
          case KEY_ENTER:
          case '\n':
            textinput = 0;
            if(strlen(textbuffer) > 0){
              chat_send(chat_server_descriptor, 0, textbuffer);
              //add_log(textbuffer,textpos);
            }
            textpos = 0;
            memset(&textbuffer, 0, BUFFER);
            break;
          // Add the character to textbuffer if we were inputting text
          default:
            if(textinput == 1) {
              textbuffer[textpos] = readc;
              textpos++;
              if(textpos == BUFFER-1) {
                textpos = 0;
                textinput = 0;
              }
            }
            break;
          }
      }

      if(FD_ISSET(game_descriptor, &readfs)) {
        struct sockaddr_storage sender_address;
        socklen_t sender_address_size = sizeof(struct sockaddr_storage);
        memset(&sender_address, 0, sizeof(struct sockaddr_storage));
        uint8_t recvbuffer[1024];
        memset(recvbuffer, 0, 1024);
        if(recvfrom(game_descriptor, &recvbuffer, 1024, 0, (struct sockaddr*)&sender_address, &sender_address_size) < 0){
          perror("recvfrom");
        }else if(recvbuffer[0] == GAME_INFO){
          playerid = recvbuffer[1];
          health   = recvbuffer[2];
          deaths = 0;
          frags = 0;
          char path[255];
          sprintf(path, "clientdata/%.*s.map", 8, &recvbuffer[3]);
          int correct_map = 0;
          //test if the map is correct
          do {
            char hash[33];
            if(md5_from_file(path, hash) == 0){//return 0 if the file exists
              if(memcmp(hash, &recvbuffer[11], 32) == 0){//test the map contents
                correct_map = 1;
              }
            }

            if(!correct_map){
              //download the map if the map is not found or the hash is wrong
              int descriptor = prepare_connection_TCP(address, map_port);
              if (descriptor > 0){
                char id[9];
                recv_map(descriptor, id);
                close(descriptor);
              }
            }
          }while(!correct_map);
          map_t map;
          read_map(path, &map);
          free_gamearea(game);
          game = new_gamearea(map.rows, map.colums, map.number_of_blocks, map.block_positions);
          send_ready(game_descriptor, (struct sockaddr*)&sender_address, sender_address_size);
          if (chat_server_descriptor == -1){
            chat_server_descriptor = prepare_connection_TCP(address, chat_port);
          }
          status = 1;
        }else if(recvbuffer[0] == SPAWN){
          if (recvbuffer[1] == 7){
            exit(1);
          }
          if (players[recvbuffer[1] - 1] == NULL){
            players[recvbuffer[1] - 1] = new_player(recvbuffer[1], recvbuffer[2], recvbuffer[3], health);
          }else{
            players[recvbuffer[1] - 1]->health = health;
            players[recvbuffer[1] - 1]->posx = recvbuffer[2];
            players[recvbuffer[1] - 1]->posy = recvbuffer[3];
          }
        }else if(recvbuffer[0] == DISCONNECT_ACK){
          free_player(players[recvbuffer[1] - 1]);
          players[recvbuffer[1] - 1] = NULL;
        }else if(recvbuffer[0] == MOVE_ACK){
          //if a new client package is lost, use the move_ack to resync
          if(players[recvbuffer[1] - 1] == NULL){
             players[recvbuffer[1] - 1] = new_player(recvbuffer[1], recvbuffer[2], recvbuffer[3], health);
          }
          players[recvbuffer[1] - 1]->posx = recvbuffer[2];
          players[recvbuffer[1] - 1]->posy = recvbuffer[3];
          players[recvbuffer[1] - 1]->hitwall = 0;
        }else if(recvbuffer[0] == WALL_ACK){
          players[playerid - 1]->health--;
          players[playerid - 1]->hitwall = 1;
        }else if(recvbuffer[0] == SUICIDE_ACK){
          players[playerid - 1]->health = 0;
          players[playerid - 1]->hitwall = 0;
          deaths++;
        }else if(recvbuffer[0] == KILLED_ACK){
          players[playerid - 1]->health = 0;
          players[playerid - 1]->hitwall = 0;
          deaths++;
        }else if(recvbuffer[0] == KILL_ACK){
          frags = recvbuffer[2];
        }else if(recvbuffer[0] == DEATH_ACK){
          players[recvbuffer[1] - 1]->health = 0;
        }else if (recvbuffer[0] == HIT_ACK){
          players[playerid - 1]->health--;
        }else if(recvbuffer[0] == PONG){
          if (ping_id == recvbuffer[1]){
            number_of_lost_pings = 0;
            struct timeval now;
            gettimeofday(&now, NULL);
            struct timeval result;
            timersub(&now, &ping_time, &result);
            latency = time_diff(&now, &ping_time);
          }
        }else if (recvbuffer[0] == MAP_CHANGE){
          sleep(2);
          clear_log();
          logposition = 0;
          number_of_lost_pings = 0;
          if(chat_server_descriptor > 0){
            close(chat_server_descriptor);
            chat_server_descriptor = -1;
          }

          for (int i = 0; i < 6; i++){
            free_player(players[i]);
            players[i] = NULL;
          }
          send_connect(game_descriptor, !spectator_mode, (struct sockaddr *)&server_sockaddr, server_addrlen);
        }
      }

      if(chat_server_descriptor != -1 && FD_ISSET(chat_server_descriptor, &readfs)) {
        char buffer[131];
        memset(buffer, 0, 131);
        int full_msg = 0;
        int msg_len = chat_recv(chat_server_descriptor, buffer, &full_msg);
        if (msg_len > 0){
          char chat_string[255];
          memset(chat_string, 0, 255);
          if (buffer[0] != 0){
            sprintf(chat_string, "Player %"SCNu8": %s", buffer[0], &buffer[2]);
          }else{
            sprintf(chat_string, "[SERVER]: %s", &buffer[2]);
          }
          add_log(chat_string, strnlen(chat_string, 255));
        }else{
          status = 0;
          close(chat_server_descriptor);
          for (int i = 0; i < 6; i++){
            free_player(players[i]);
            players[i] = NULL;
          }
          chat_server_descriptor = -1;
        }
      }
    }

    if(status && number_of_lost_pings > 2){
      status = 0;
      close(chat_server_descriptor);
      for (int i = 0; i < 6; i++){
        free_player(players[i]);
        players[i] = NULL;
      }
      chat_server_descriptor = -1;
    }

    struct timeval now;
    gettimeofday(&now, NULL);
    double ms = time_diff(&now, &ping_time);
    if (status && ms > 1000){
      ping_id = send_ping(game_descriptor, (struct sockaddr *)&server_sockaddr, server_addrlen);
      gettimeofday(&ping_time, NULL);
      number_of_lost_pings++;
    }
    // Update screen
    ui_draw_grid(game, playerid, players);

    // Surrended
    if(quit) {
      ui_draw_end(0);
      break;
    }
  }
  for (int i = 0; i < 6; i++){
    free_player(players[i]);
    players[i] = NULL;
  }
  free_gamearea(game);
  free_horizontal_line();
  sleep(1);
  endwin(); // End ncurses
  return 0;
}

int main(int argc, char **argv)
{
  int optc = -1;
  char *port = NULL;
  char *addr = NULL;

  if (argc < 2){ //-p27015 are two argvs
      help(argv[0]);
      return 0;
  }

  while ((optc = getopt(argc, argv, "h:p:")) != -1) {
      switch (optc) {
          case 'h':
              addr = optarg;
              break;
          case 'p':
          {
              int port_number;
              if (is_number(optarg, 10, &port_number)){
                  port = optarg;
              }else{
                  printf("Invalid port number: %s\n", optarg);
              }
          }
              break;
          case ':':
              printf ("Something?\n");
              break;
          case '?':
              switch(optopt){
                  case 'p':
                  case 'l':
                      printf("-%c: Missing port.", optopt);
                      break;
                  case 'h':
                      printf("-h: Missing IP address.\n");
                      break;
              }
              break;
      }
  }

  if (addr != NULL && port != NULL){
    client(addr, atoi(port));
  }else{
    help(argv[0]);
  }
  return 0;
}


double time_diff(struct timeval *after, struct timeval *before)
{
    double delta = after->tv_usec/1000.0 + 1000 * after->tv_sec - (before->tv_usec / 1000.0 + 1000 * before->tv_sec);
    return delta;
}
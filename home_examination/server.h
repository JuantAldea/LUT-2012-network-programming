/*
###############################################
#        CT30A5001 - Network Programming      #
#               Home Examination              #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                  Makefile                   #
###############################################
*/

#ifndef __SERVER_H__
#define __SERVER_H__

#include "system_headers.h"
#include "linked_list.h"
#include "map.h"
#include "tcp.h"
#include "udp.h"
#include "map_server.h"
#include "game_server.h"
#include "chat_server.h"
#include "game_protocol.h"
#include "chat_protocol.h"
#include <signal.h>
void kick_players(linked_list_t *player_list);
void server(int port);
#endif

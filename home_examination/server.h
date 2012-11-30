#ifndef __SERVER_H__
#define __SERVER_H__

#include "system_headers.h"
#include "linked_list.h"
#include "map.h"
#include "tcp.h"
#include "udp.h"
#include "map_server.h"
#include "game_server.h"

void print_mapcycle(linked_list_t *list);
void parse_mapcycle(linked_list_t **list);
void delete_mapcycle_list(linked_list_t **list);

#endif

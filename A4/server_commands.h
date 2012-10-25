/*
####################################################
#         CT30A5001 - Network Programming          #
# Assignment 4: Multicast Game announcement system #
#                        &                         #
#                   tic-tac-toe                    #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                 sever_commands.h                 #
####################################################
*/

#ifndef __SERVER_COMMANDS_H__
#define __SERVER_COMMANDS_H__

#define SEARCH_COMMAND_STR  "/search"
#define GAMES_COMMAND_STR   "/games"
#define JOIN_COMMAND_STR    "/join"
#define GRID_COMMAND_STR    "/grid"
#define PLACE_COMMAND_STR   "/place"
#define QUIT_COMMAND_STR    "/quit"


#define SEARCH_COMMAND_CODE 0
#define GAMES_COMMAND_CODE  1
#define JOIN_COMMAND_CODE   2
#define GRID_COMMAND_CODE   3
#define PLACE_COMMAND_CODE  4
#define QUIT_COMMAND_CODE   5

#define UNKOWN_COMMAND_CODE     255

#endif

/*
###############################################
#        CT30A5001 - Network Programming      #
#          Assignment 6: FTP Client           #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#               client_states.h               #
###############################################
*/

#ifndef __CLIENT_STATES_H__
#define __CLIENT_STATES_H__

enum states
{
    NOT_CONNECTED = 0,
    CONNECTED,
    LOGGED_IN
};

enum transfer_modes
{
    ACTIVE = 0,
    PASSIVE
};

enum transfer_types
{
    NONE = 0,
    LS,
    SEND,
    RECV
};

#endif
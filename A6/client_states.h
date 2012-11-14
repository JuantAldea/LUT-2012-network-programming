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
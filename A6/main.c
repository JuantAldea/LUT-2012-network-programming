/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                   main.c                    #
###############################################
*/

#include "common.h"
#include "client.h"

int is_number(char *str, int base, int *number);

void help(char *name);

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
                    case 'n':
                        printf("-n: Missing name.\n");
                        break;
                }
                break;
        }
    }
    if (port != NULL && addr != NULL){
        client(addr, port);
    }else{
        printf("PROBLEM WITH PARAMS\n");
    }
    return 0;
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

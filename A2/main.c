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
#include "server.h"
#include "client.h"

int is_number(char *str, int base, int *number);

void help(char *name);

int main(int argc, char **argv)
{
	int server_mode = 0;
	int client_mode = 0;

	int optc = -1;
	char *name = NULL;
	char *port = NULL;
	char *addr = NULL;

	if (argc < 2){ //-p27015 are two argvs
		help(argv[0]);
		return 0;
	}
	int port_number;
	while ((optc = getopt(argc, argv, "l:h:p:n:")) != -1) {
		switch (optc) {
			case 'l':
				if (is_number(optarg, 10, &port_number)){
					port = optarg;
					printf("Port number: %d\n", port_number);
				}else{
					printf("Invalid port number: %s\n", optarg);
				}
				server_mode = 1;
				break;
			case 'h':
				addr = optarg;
				client_mode = 1;
				break;
			case 'p':
				if (is_number(optarg, 10, &port_number)){
					port = optarg;
					printf("Port number: %d\n", port_number);
				}else{
					printf("Invalid port number: %s\n", optarg);
				}
				break;
			case 'n':
				name = optarg;
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

	if (server_mode && client_mode){
		printf("Wrong sintax\n");
		help(argv[0]);
	}else if (server_mode){
		if (port != NULL){
			return server(port);
		}else{
			printf("Listening port need: -l <port>\n");
		}
	}else if (client_mode){
		int missing_params = 0;
		if (addr == NULL){
			printf("Server address need: -h <address>\n");
			missing_params = 1;
		}
		if(port == NULL){
			printf("Remote port need: -p <port>\n");
			missing_params = 1;
		}
		if (name == NULL){
			printf("Name need: -n <name>\n");
			missing_params = 1;
		}
		if (!missing_params){
			printf("%s %s\n", addr, port);
			return client(name, addr, port);
		}
	}else{
		printf("Wrong sintax\n");
		help(argv[0]);
	}
	return 0;
}

void help(char *program){
	printf("Server mode: %s -l <listening port>\n", program);
	printf("Client mode: %s -h <host address> -p <port> -n <name>\n", program);
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

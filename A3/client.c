/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                   client.c                  #
###############################################
*/
#include "common.h"

int client(char *command, char *address, char *port)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_protocol = IPPROTO_UDP;


	int error = 0;
	struct addrinfo *res = NULL;
	if ((error = getaddrinfo(address, port, &hints, &res)) < 0){
		printf("Getaddrinfo error: %s\n", gai_strerror(error));
		freeaddrinfo(res);
		return EXIT_FAILURE;
	}

	int socket_descriptor = -1;
	struct addrinfo *ptr;
	//loop through the address looking for one that works
	for(ptr = res; ptr != NULL; ptr = ptr->ai_next) {
        if ((socket_descriptor = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) < 0) {
        	printf("socket failed %s\n", strerror(errno));
            continue;
        }
        break;
    }

    if (ptr == NULL){
	    freeaddrinfo(res);
	    printf("error\n");
    	return -1;
    }
	struct timeval timeout;
	memset(&timeout, 0, sizeof(struct timeval));
  	timeout.tv_sec = 1;

	if (setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, &timeout,  sizeof(timeout))){
		perror("Error Setting the timeout");
		return EXIT_FAILURE;
	}

	char recvbuffer[512];
	struct sockaddr_storage client_addr;
	socklen_t address_len = sizeof(client_addr);

	if (!strncmp(command, "ADD", 3)){
		char aphorism_buffer[509];
		memset(aphorism_buffer, 0, 509);

		if (NULL == fgets(aphorism_buffer, 508, stdin)){
			printf("[ERROR] fgets failed\n");
		}
		flush_stdin();
		int aph_len = strlen(aphorism_buffer);
		if (aph_len < 0){
			freeaddrinfo(res);
			printf("[ERROR] Aphorism too short\n");
			return 0;
		}else{
			aphorism_buffer[aph_len - 1] = '\0';
		}
		send_ADD(socket_descriptor, ptr->ai_addr, ptr->ai_addrlen, aphorism_buffer);
		memset(recvbuffer, 0, 512);
    	if(recvfrom(socket_descriptor, recvbuffer, 512, 0, (struct sockaddr*)&client_addr, &address_len)>= 0){
	   		if (!strncmp(recvbuffer, "AOK", 3)){
	   			printf("[SERVER] Aphorism added!\n");
	   		}else if (!strncmp(recvbuffer, "ERR", 3)){
				printf("[ERROR] %s\n", recvbuffer + 3);
	   		}
    	}else{
    		perror("[ERROR]");
    	}


	}else if (!strncmp(command, "GET", 3)){
		send_GET(socket_descriptor, ptr->ai_addr, ptr->ai_addrlen);
		memset(recvbuffer, 0, 512);
		if(recvfrom(socket_descriptor, recvbuffer, 512, 0, (struct sockaddr*)&client_addr, &address_len)>= 0){
			if (!strncmp(recvbuffer, "APH", 3)){
					printf("[APH]%s\n", recvbuffer + 3);
				}else if (!strncmp(recvbuffer, "ERR", 3)){
				printf("[ERROR] %s\n", recvbuffer + 3);
				}
		}else{
			perror("[ERROR]");
		}
	}

    freeaddrinfo(res);
	close(socket_descriptor);
	return EXIT_SUCCESS;
}


int is_number(char *str, int base, int *number);
void help_client(char *command);

int main(int argc, char **argv)
{
	int optc = -1;
	char *command = NULL;
	char *port = NULL;
	char *addr = NULL;
	int port_number;

	while ((optc = getopt(argc, argv, "h:p:c:")) != -1) {
		switch (optc) {
			case 'h':
				addr = optarg;
				break;
			case 'p':
				if (is_number(optarg, 10, &port_number)){
					port = optarg;
					printf("Port number: %d\n", port_number);
				}else{
					printf("Invalid port number: %s\n", optarg);
				}
				break;
			case 'c':
				command = optarg;
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
						printf("-n: Missing command.\n");
						break;
				}
				break;
		}
	}

	int missing_params = 0;
	if (addr == NULL){
		printf("Server address need: -h <address>\n");
		missing_params = 1;
	}
	if(port == NULL){
		printf("Remote port need: -p <port>\n");
		missing_params = 1;
	}
	if (command == NULL){
		printf("Command need: -n <command>\n");
		missing_params = 1;
	}
	if (!missing_params){
		return client(command, addr, port);
	}else{
		help_client("Instructions: ");
	}

	return 0;
}

void help_client(char *program){
	printf("%s -h <host address> -p <port> -c <command>\n", program);
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

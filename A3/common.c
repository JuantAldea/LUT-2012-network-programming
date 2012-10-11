#include "common.h"

void flush_stdin(void)
{
	int bytes_in_stdin = 0;
	ioctl(STDIN_FILENO, FIONREAD, &bytes_in_stdin);
	while(bytes_in_stdin){
		char *garbage = (char *)malloc(sizeof(char) * bytes_in_stdin);
		if (NULL == fgets(garbage, bytes_in_stdin, stdin)){
			printf("[ERROR] fgets failed\n");
		}
		free(garbage);
		ioctl(STDIN_FILENO, FIONREAD, &bytes_in_stdin);
	}
}

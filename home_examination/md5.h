#ifndef __MD5_H__
#define __MD5_H__

void md5_from_file(char *filepath, char hash[32]);
char* md5(char * initial_msg, size_t initial_len);

#endif

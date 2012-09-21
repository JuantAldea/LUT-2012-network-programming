typedef struct message message;

struct message{
	int size;
	int type;
	char *nickname;
	char *message;
}

void message_init(message *msg)
{
	msg->size = -1;
	msg->type = -1;
	if (msg->nickname != NULL){
		free(msg->nickname);
		msg->nickname = NULL;
	}
	if (msg->message !=NULL){
		free(msg->message);
		msg->message = NULL;
	}
}

void message_free(message *msg)
{
	message_init(msg);
}

void message_fill(int size, int type, char *nickname, char *message, message *msg)
{
	msg->size = size;
	msg->type = type;
	if (nickname != NULL){
		msg->nickname = (char *) malloc(sizeof(char) * 16);
		memset(msg->nickname, '\0', sizeof(char) * 16);
	}
	msg->message = (char *) malloc(sizeof(char) * 16);
	memset(msg->nickname, '\0', sizeof(char) * 16);
	memcpy(msg->nickname, nickname, sizeof(char) * 15);
	mem
}
typedef struct user_node user_node;

struct user_node{
	int fd;
	char *name;
}

user_node_init(user_node *node);
user_node_free(user_node *node);
user_node_fill(user_node *node);
user_node_set_name(char *name, user_node *node);
user_node_compare_id(user_node *a, user_node *b);
user_node_compare_name(user_node *a, user_node *b);



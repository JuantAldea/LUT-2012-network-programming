

int server(char *port)
{
    linked_list_t *chat_clients = (linked_list_t*)malloc(sizeof(linked_list_t));
    list_init(chat_clients);

    int chat_server_socket = -1;
    if ((chat_server_socket = prepare_server(port)) < 0){
        exit(EXIT_FAILURE);
    }

    struct sockaddr_storage remote_addr;
    socklen_t addr_size;
    addr_size = sizeof(remote_addr);

    fd_set descriptors_set;
    FD_ZERO(&descriptors_set);
    FD_SET(STDIN_FILENO, &descriptors_set);
    FD_SET(chat_server_socket, &descriptors_set);

    int max_fd = chat_server_socket;
    int running = 1;//server running
    int server_full = 0;
    while(running){
        if(select(max_fd + 1, &descriptors_set, NULL, NULL, NULL) < 0) {
            perror("Error in select");
            exit(EXIT_FAILURE);
        }

        //check activity in the listening socket
        if (FD_ISSET(chat_server_socket, &descriptors_set)){
            //we have an incomming connection
            int chat_new_connection_socket = -1;
            if ((chat_new_connection_socket = accept(chat_server_socket, (struct sockaddr*)&remote_addr, &addr_size)) < 0){
                printf("[NEW CONNECTION] Error in the incoming connection %s\n", strerror(errno));
            }
            //if we are accepting new connections, register the new user
            if (!server_full){
                printf("[NEW CONNECTION] New client connected with fd = %d\n", chat_new_connection_socket);
                node_t *new_user = list_create_node(chat_new_connection_socket, unknow_name);
                list_add_last(new_user, chat_clients);
            }else{
                //if we are not, drop the connection.
                char error_msg[] = "Game full";
                send_error(chat_new_connection_socket, error_msg);
                close(chat_new_connection_socket);
            }
        }

        //check activity in every client descriptor
        for(node_t *i = chat_clients->head->next; i != chat_clients->tail; i = i->next){
            if (FD_ISSET(i->client->fd, &descriptors_set)){
                //if we read, we need to know if we received the whole msg
                int full_message = 0;
                int bytes_readed = recv_msg(i->client->fd, i->buffer, &full_message);
                //if we read 0 bytes, we have a disconnect
                if (bytes_readed <= 0){
                    //the current node is about to be removed
                    //so next iteration needs an small ajustment
                    node_t *previous = i->previous;
                    broadcast_quit_message(i->client->name, "Player X disconnected", chat_clients);
                    manage_disconnect_by_node(i, chat_clients, 0);
                    i = previous;
                }else if (full_message){//we received the whole msg
                    printf("[CHAT] <%s> %s\n", i->client->name, i->buffer->buffer);
                    broadcast_chat_message(i->client->name, (char*)i->buffer->buffer, chat_clients);
                    recv_buffer_reset(i->buffer);
                }
            }
        }

        //reset the set
        FD_ZERO(&descriptors_set);
        FD_SET(chat_server_socket, &descriptors_set);
        //also set the max descriptor
        max_fd = chat_server_socket;
        for(node_t *i = chat_clients->head->next; i != chat_clients->tail; i = i->next){
            FD_SET(i->client->fd, &descriptors_set);
            max_fd = ((i->client->fd > max_fd) ? i->client->fd : max_fd);
        }
    }

    //at the end, disconnect everyone, clear all.
    disconnect_clients(chat_clients);
    close(chat_server_socket);
    list_delete(chat_clients);//clear list
    free(chat_clients);
    return 0;
}

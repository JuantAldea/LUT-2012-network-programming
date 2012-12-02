/*
###############################################
#        CT30A5001 - Network Programming      #
#        Assignment2: TCP multiuser chat      #
#   Juan Antonio Aldea Armenteros (0404450)   #
#        juan.aldea.armenteros@lut.fi         #
#                 linked_list.c               #
###############################################
*/

#include "linked_list.h"

//set-up the head and tail of the list
void list_init(linked_list_t *list)
{
    list->count = 0;
    list->head = (node_t *)malloc(sizeof(node_t));
    list->tail = (node_t *)malloc(sizeof(node_t));
    list->tail->data = NULL;
    list->head->data = NULL;
    list->head->next = list->tail;
    list->head->previous = NULL;
    list->tail->next = NULL;
    list->tail->previous = list->head;
}

//free the list
void list_delete(linked_list_t *list)
{
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        node_t *previous = i->previous;
        list_remove_node(i, list);
        i = previous;
    }
    free(list->head);
    free(list->tail);
}

//build a new list node
node_t *list_create_node(void *data)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    node->data = data;
    return node;
}

//add node after the head
void list_add_first(node_t *node, linked_list_t *list)
{
    list->count++;
    node->next = list->head->next;
    node->previous = list->head;
    list->head->next->previous = node;
    list->head->next = node;
}

//add node after the tail
void list_add_last(node_t *node, linked_list_t *list)
{
    list->count++;
    node->next = list->tail;
    node->previous = list->tail->previous;
    list->tail->previous->next = node;
    list->tail->previous = node;
}

//remove node from the list
void list_remove_node(node_t *node, linked_list_t *list)
{
    assert(node != list->head);
    assert(node != list->tail);
    list->count--;
    node->previous->next = node->next;
    node->next->previous = node->previous;
    free(node->data);
    free(node);
}

void list_print(linked_list_t *list)
{
    printf("########################## Clients #######################\n");
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        //printf("%d: %s\n", i->client->fd, i->client->name);
    }
    printf("##########################################################\n");
}

void list_reverse_print(linked_list_t *list)
{
    printf("########################## Clients #######################\n");
    for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
        //printf("%d: %s\n", i->client->fd, i->client->name);
    }
    printf("##########################################################\n");
}

node_t *list_search_by_addrinfo(struct sockaddr_storage *addr, linked_list_t *list)
{
    for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
        player_info_t *info = (player_info_t *)i->data;
        if(!memcmp(&info->addr, addr, sizeof(struct sockaddr_storage))){
            return i;
        }
    }
    return NULL;
}

node_t *list_search_by_addr(struct sockaddr_storage *addr, linked_list_t *list)
{
    for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
        player_info_t *info = (player_info_t *)i->data;
        if (addr->ss_family == info->addr.ss_family){
            if (addr->ss_family == AF_INET){
                if(!memcmp(&((struct sockaddr_in*)&info->addr)->sin_addr,
                    &((struct sockaddr_in*)addr)->sin_addr,
                     sizeof(struct in_addr))){
                    return i;
                }
            }else if (addr->ss_family == AF_INET6){
                if(!memcmp(&((struct sockaddr_in6*)&info->addr)->sin6_addr,
                    &((struct sockaddr_in6*)addr)->sin6_addr,
                     sizeof(struct in6_addr))){
                    return i;
                }
            }
        }else{
            printf("family\n");
        }
    }
    return NULL;
}

node_t *list_search_by_playerID(uint8_t id, linked_list_t *list)
{
    for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
        player_info_t *info = (player_info_t *)i->data;
        if (info->playerID == id){
            return i;
        }
    }
    return NULL;
}

void list_sort_by_frags(linked_list_t *list)
{
    int count = list->count;

    node_t *highest = list->head->next;

    for(node_t *i = highest; i != list->tail; i = i->next){
        if (((player_info_t *)(highest->data))->frags < ((player_info_t *)(i->data))->frags){
            highest = i;
        }
    }
    //we need to keep the supremun of the set, because it's the start of the unsorted part of the list
    //unlink
    highest->previous->next = highest->next;
    highest->next->previous = highest->previous;
    //store the supremum in the first position
    list_add_first(highest, list);
    list->count--;
    count--;//one is in it's place

    while(count > 0){
        node_t *remaining_max = highest->next;
        //form the first unordered
        for(node_t *i = remaining_max; i != list->tail; i = i->next){
            //find the maximum
            if (((player_info_t *)(remaining_max->data))->frags < ((player_info_t *)(i->data))->frags){
                remaining_max = i;
            }
        }
        assert(remaining_max != list->tail);
        assert(remaining_max != list->head);
        //unlink the max
        remaining_max->previous->next = remaining_max->next;
        remaining_max->next->previous = remaining_max->previous;
        //and place it at the start of the list
        list_add_first(remaining_max, list);
        list->count--;
        count--;
    }
}
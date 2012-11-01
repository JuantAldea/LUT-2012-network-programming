/*
####################################################
#         CT30A5001 - Network Programming          #
#               Assignment 5: SCTP                 #
#      Juan Antonio Aldea Armenteros (0404450)     #
#           juan.aldea.armenteros@lut.fi           #
#                  linked_list.c                   #
####################################################
*/

#include "linked_list.h"

//set-up the head and tail of the list
void list_init(linked_list_t *list)
{
    list->count = 0;
    list->head = (node_t *)malloc(sizeof(node_t));
    list->tail = (node_t *)malloc(sizeof(node_t));
    list->head->next = list->tail;
    list->head->previous = NULL;
    list->tail->next = NULL;
    list->tail->previous = list->head;
}

//free the list
void list_delete(linked_list_t *list)
{
    list_clear(list);
    free(list->head);
    free(list->tail);
}

void list_clear(linked_list_t *list)
{
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        node_t *previous = i->previous;
        list_remove_node(i, list);
        i = previous;
    }
}

//build a new list node
node_t *list_create_node(int8_t player_id, sctp_assoc_t session_id, struct sockaddr_storage sa)
{
    node_t *node = (node_t *)malloc(sizeof(node_t));
    memcpy(&node->sa, &sa, sizeof(struct sockaddr_storage));
    node->session_id = session_id;
    node->player_id = player_id;
    node->turn = -1;
    node->ready = 0;
    node->next = NULL;
    node->previous = NULL;
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
    free(node);
}

node_t* list_get_node_by_index(int index, linked_list_t *list)
{
    assert(index < list->count);
    assert(index >= 0);
    node_t *i = list->head;

    for (int j = 0; j <= index; j++){
        i = i->next;
    }

    return i;
}


node_t *list_get_node_by_player_id(int8_t player_id, linked_list_t *list)
{
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        if (i->player_id == player_id){
            return i;
        }
    }

    return NULL;
}

node_t *list_get_node_by_session_id(sctp_assoc_t session_id, linked_list_t *list)
{
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        if (i->session_id == session_id){
            return i;
        }
    }

    return NULL;
}

int8_t list_get_first_free_player_id(linked_list_t *list)
{
    if (list->count == 0){
        return 1;
    }

    if (list->count == 4){
        return -1;
    }

    int8_t ids_in_use[4];
    memset(ids_in_use, 0, sizeof(int8_t) * 4);

    //get the ids in use
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        ids_in_use[i->player_id - 1] = 1;
    }

    //return the first free id
    for (int i = 0; i < 4; i++){
        if (!ids_in_use[i]){
            return i+1;
        }
    }

    //no free id
    return -1;
}

void list_print(linked_list_t *list)
{
    printf("\n########################## Players #######################\n");
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        printf("Player ID: %d Association ID: %d Turn: %d Ready: %d\n", i->player_id, i->session_id, i->turn, i->ready);
    }
    printf("############################################################\n");
}

void list_reverse_print(linked_list_t *list)
{
    printf("\n########################## Aphorisms #######################\n");
    for(node_t *i = list->tail->previous; i != list->head; i = i->previous){
    }
    printf("############################################################\n");
}


void list_sort_by_turn(linked_list_t *list)
{
    int count = list->count;

    node_t *highest = list->head->next;

    for(node_t *i = highest; i != list->tail; i = i->next){
        if (highest->turn < i->turn){
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
            if (remaining_max->turn < i->turn){
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

int list_ready_check(linked_list_t *list)
{
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        if (!i->ready){
            return 0;
        }
    }

    return 1;
}

void list_set_movement_order(linked_list_t *list)
{
    int turns[list->count];
    int8_t assigned_turns[list->count];
    memset(assigned_turns, 0, sizeof(int8_t) * list->count);
    srand(time(NULL));
    int current_player = 0;

    while(current_player < list->count){
        int current_turn = rand() % list->count;
        if (assigned_turns[current_turn] == 0){
            assigned_turns[current_turn] = 1;
            turns[current_turn] = current_player;
            current_player++;
        }
    }

    int index = 0;
    for(node_t *i = list->head->next; i != list->tail; i = i->next){
        i->turn = turns[index];
        index++;
    }
}
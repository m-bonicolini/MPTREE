#ifndef L_LIST_H
#define L_LIST_H
#endif

#include "queue.h"

#ifdef PROXY_C89 



typedef queue l_list;

#else

#error compiler is not ANSI C89

#endif

#if defined(PROXY_C89) && defined(QUEUE_H)

extern l_list* new_linked(l_list *ls);/*create new linked list*/
extern int add_item(l_list *ls,void* data,int size);/*push item in list*/
extern l_list* add_to(l_list *ls,void* data,int size,int index);/* add item at index position*/
extern l_list* delete_linked(l_list *head);/*delete the whole linked list*/
extern l_list* get_linked(int index,l_list *ls);/*get node at position index*/
extern l_list * delete_linked_node(l_list *head,int index);/*delete linked node at index position*/
extern int is_empty_linked(l_list *ls);/*check if list is empty*/
extern l_list* delete_to(int start,int end,l_list* head);/*delete all nodes betwenn start and end position*/
extern int add_node(l_list *head, l_list *node);/*add node to linked*/
extern l_list* add_node_index(l_list *head,l_list *node,int index);/*add node to linked at position index*/
extern void print_linked(l_list *head);
extern int add_data_linked(l_list *head,int index,void* data,int size);/*add data to object in index position*/
extern l_list* go_next(l_list *ls);

#else

#error l_list.h not defined or compiler is not ANSI C89

#endif

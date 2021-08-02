#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Message.h"

#ifndef QUEUE_H

#define QUEUE_H



struct q_node
{
	/*pointer to next queue-node*/
	struct q_node *next;

	/*pointer to item contained in queue-node*/
	void *item;
	unsigned short int i_size;
	unsigned short int a_size;
};

typedef struct q_node queue;

extern queue* new_queue(queue*q);/* create a new empty queue*/
extern int is_empty_q(queue* q);/* check if queue is empty*/
extern queue* cpy_pop(void* item,queue* q,int *check,int size);/*pop node and cpy it in "item"*/
extern int push(void *elemento, queue *q,int size);/*cpy "elemento" in a new queue node*/
extern int point_push(void *elemento, queue *q);/*add elemento in queue*/
extern int q_length(queue *q);/*return q length*/
extern void*  delete_q(queue* q);/*delete all queue*/
extern void print_node(queue* q,const int flag);
extern void print_queue(queue*q);
extern int get_isize(queue* q);
extern int get_asize(queue*q);
extern queue* pop_s_item(queue*q);/*pop queue node but not free item*/
extern void* point_item(void *p, queue* q,int *myerror);
extern queue* add_to_head(queue* q, void* item,const int size);
extern int add_to_item(queue* q, void *data,int size);
extern queue* delete_item(int size,queue* q);

#endif


#include "queue.h"

#ifndef QUEUE_H
#error NO queue.h header
#endif

#ifndef QUEUE_C
#define QUEUE_C
#endif

#if (defined(PROXY_C89))
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 







/*struct of queue-node*/

/*new_queue(pointer)->address of memory
	return address where is allocate new queue-node*/
 queue* new_queue(queue*q)
{
	if(q!=NULL) e_fatal("assigned pointer",0);


#if defined(PROXY_ILP64) || defined(PROXY_LP64)/*64bit cpu*/
		q=(queue*)malloc(20);
#else
    #if defined(PROXY_LP32)||defined(PROXY_ILP32) /*32bitcpu*/
		q=(queue*)malloc(12);
    #else
    #error unknow or unsupported architeture(16 bit cpu or slower or better than 64 bit cpu or a proxy_unknow arch)
    #endif
#endif

	/*no memory allocated malloc failed.*/
	if(q==NULL) e_fatal("malloc failed",0);

	#if defined(PROXY_ILP64) || defined(PROXY_LP64)
    memset(q,0x00,20);
#else
  #if defined(PROXY_LP32)||defined(PROXY_ILP32) /*32bitcpu*/
    memset(q,0x00,12);
  #else
  #error unknow or unsupported architetture
  #endif
#endif
    q->next=NULL;
	q->item=NULL;
	memset(&(q->i_size),0x00,2);
	memset(&(q->a_size),0x00,2);
	return q;
}

/* check if queue is empty*/
int is_empty_q(queue* q)
{
	if(q==NULL) return -1;/*error*/
	else
	{
		if(q->item==NULL && q->next==NULL && q->i_size==0 && q->a_size==0)
		return 1;/*is empty*/
		else return 0;/*is not empty*/
	}
}
/*pop queue-node and copy q-node->item in elemento.
	store in var value that check if function terminated with no error*/
static queue* pop_i(void *elemento,int size, queue *q,int *myres)
{
	queue* tmp=NULL;

	if(myres==NULL)/*you have not pass res*/
	e_fatal("myres must be !=NULL",0);
	else memset(myres,0x00,sizeof(int));/*clear res*/


	if(q==NULL)/*you have pass a NULL or pointer to NULL*/
	{
		*myres=2;
		return NULL;
	}
	if(elemento==NULL) /*i can not store q->item in elemento*/
	{
		*myres=3;
		return q;
	}

    if(size<q->i_size)/*i can not store q->item in elemento*/
    {
        *myres=4;
        return q;
    }

	if( is_empty_q(q))/*queue is empty*/
	{
#ifdef PROXY_DEBUG
		e_warning("There are no object to pop",0);
		e_warning("if you want delete queue use delete_q()",0);
#endif
        *myres=5;
		return q;
	}
	else
	{

		if(q->next!=NULL) /*there is more than one q-node*/
		{
			tmp=q;
			memset(elemento,0x00,size);/*clear elemento*/

            if(q->item!=NULL)/*there is something*/
			memcpy(elemento,q->item,q->i_size);/*copy item*/
            else/*there is not any item*/
            {
                *myres=6;
                return q;
            }

			q=q->next;/*switch head*/

            memset(tmp->item,0x00,tmp->a_size);/*clear item before free*/
			free(tmp->item);
			tmp->item=NULL;
			tmp->next=NULL;
			memset(&(tmp->i_size),0x00,2);
			memset(&(tmp->a_size),0x00,2);
			free(tmp);
			tmp=NULL;
			*myres=0;
			return q;
		}
		else/*only one q-node*/
		{
            memset(elemento,0x00,size);
            if(q->item!=NULL)/*there is something*/
			memcpy(elemento,q->item,q->i_size);/*copy item*/
            else/*there is not any item*/
            {
                *myres=6;
                return q;
            }
			memset(&(q->i_size),0x00,2);
			memset(&(q->a_size),0x00,2);

            memset(q->item,0x00,q->a_size);/*clear item up free*/
            free(q->item);
			q->item=NULL;/*q->item point NULL*/

            *myres=0;/*all ok queue is empty for delete queue use delelte_queue()*/
			return q;
		}
	}
}
/*execute i_pop*/
 queue* cpy_pop(void* item,queue* q,int *check,const int size)
{
		int myres=0;
		/*int myerror=0;*/
		if(check==NULL) e_fatal("check must be !=NULL",0);
#if defined(PROXY_ILP32) || defined(PROXY_LP64)
		memset(check,0x00,4);
#else
	#ifdef PROXY_LP32
		memset(check,0x00,2);
	#elif defined(PROXY_ILP64)
		memset(check,0x00,8);
	#elif
		#error unknow architeture
	#endif
#endif
		q=pop_i(item,size,q,&myres);
		/*myerror=errno;*/
		if(myres)
		{
			switch(myres)
			{
				case 5:
				{
					e_warning("queue is empty",0);
					*check=1;
					return q;
				}break;
				case 6:
				{
					e_warning("pushed item is NULL this is an anomalous node",0);
					*check=2;
					return q;
				}break;
				default:
				{
					e_fatal("wrong args",0);
				}break;
			}
		}/*if(myres<0 || myerror>0)*/
		return q;
}

/*add elemento in queue in a new queue node
	is a tail-recursive function*/
int push(void *elemento, queue *q,int size)
{
	if(size<=0 || size>2048) return 0;
	if(elemento==NULL || q==NULL) return 0;/*you have pass no item to add or there is not any queue*/
	else
	{
		if(q->next!=NULL) return push(elemento,q->next,size);/*i'm not the end*/
		else/*i'm the end*/
		{
			if(q->item!=NULL)/*node is the end of queue but is not empty */
			{
				q->next=new_queue(q->next);/*next point new node  */
				return push(elemento,q->next,size);/*recall push*/
			}
			else/*node empty */
			{
				/*now i normalize size for a_size of item*/

				if(size%2!=0)q->a_size=size+1;/*if is not multiple of 2*/
				else q->a_size=size;/*is multiple of 2*/

				/*i have set size for malloc item*/
				q->item=malloc(q->a_size);/*allocated value store in a_size*/
				if(q->item==NULL)e_fatal("malloc failed",0);
				memset(q->item,0x00,q->a_size);/*clear allocated memory*/
				q->i_size=size;/*set "true" size of item*/
				memcpy(q->item,elemento,q->i_size);/*copy elemento  in q->item*/
				return 1;
			}
		}
	}
}


int point_push(void *elemento, queue *q)
{
	if(elemento==NULL || q==NULL) return 0;/*you have pass no item to add or there is not any queue*/
	else
	{
		if(q->next!=NULL) return point_push(elemento,q->next);/*i'm not the end*/
		else/*i'm the end*/
		{
			if(q->item!=NULL)/*node is the end of queue but is not empty */
			{
				q->next=new_queue(q->next);/*next point new node  */
				return point_push(elemento,q->next);/*recall push*/
			}
			else/*node empty */
			{
				q->a_size=sizeof(void*);
				q->i_size=sizeof(void*);
				q->item=elemento;
				return 1;
			}
		}
	}
}



/*estimate length of queue
	is a tail */
static int queue_length(queue* q,int res)
{
	if(q==NULL) return -1;

	if(is_empty_q(q))/*queue is empty*/
	return res;
	else
	{
		if(q->next!=NULL)/*is not the end of queue*/
		return queue_length(q->next,res+1);
		else return res+1;/*end of queue*/
	}
}
/*function using queue_length*/
int q_length(queue *q)
{
	int res=0;

	if((res=queue_length(q,0))<0)
	{
		e_fatal("NULL pointer to function q_length()",0);
		return -1;
	}
	else return res;
}


/*delete all item in a queue.is a recursive function*/
void*  delete_q(queue* q)
{
	queue* p=NULL;

	if(q==NULL) return NULL;
	do
	{
		if(q->item!=NULL)
		{
			memset(q->item,0x00,q->a_size);
			free(q->item);
			q->item=NULL;
		}
		memset(&(q->i_size),0x00,2);
		memset(&(q->a_size),0x00,2);
		p=q;
		q=q->next;
		p->next=NULL;
		free(p);
		p=q;
	}while(p!=NULL);
	return q;
}
/*print queue-node and if flag is 1 print contenent of item*/
void print_node(queue*q,const int flag)
{
	int i=0;
	byte *n=NULL;

	if(flag <0 || flag >1)e_fatal("flag must be 1 or 0",0);
	if(q==NULL)
	{
		printf(RED"NULL \n"NORMAL);
		fflush(stdout);
	}
	else
	{
		if(is_empty_q(q))
		{
			printf(RED"EMPTY \n"NORMAL);
			fflush(stdout);
			return;
		}
		memset(&i,0x00,sizeof(int));

		printf(RED"[start node]\n\n"NORMAL);
		fflush(stdout);

		printf(GREEN"node address is %p\n"NORMAL,(void*)q);
		fflush(stdout);

		printf(GREEN"next address is %p\n"NORMAL,(void*)&(q->next));
		fflush(stdout);

		printf(GREEN"next point to %p\n"NORMAL,(void*)q->next);
		fflush(stdout);

		printf(GREEN"item allocated size=%d \n"NORMAL,q->a_size);
		fflush(stdout);

		printf(GREEN"item true size=%d\n"NORMAL,q->i_size);
		fflush(stdout);

		printf(GREEN"item address is %p\n"NORMAL,(void*)&(q->item));
		fflush(stdout);

		printf(GREEN"item point  %p\n"NORMAL,(q->item));
		fflush(stdout);
		if(flag)
		{
			if(q->a_size!=0)
			{
				printf(GREEN"item is \n"NORMAL);
				fflush(stdout);
				if(q->item!=NULL)
				{
					n=(byte*)q->item;
					for(i=0;i<(q->a_size);i++)
					{
						printf("%u ",(byte)n[i]);
						fflush(stdout);
					}
					printf("\n\n");
					fflush(stdout);
					printf(RED"[end node]\n"NORMAL);
					fflush(stdout);
					n=NULL;
					return;
				}
				else
				{
					printf("%p \n",q->item);
					fflush(stdout);
				}
			}
			else return;
		}
		else return;
	}
}
/*print all node in queue*/
void print_queue(queue*q)
{
	queue *now=NULL;

	printf(RED"START OF QUEUE \n"NORMAL);
	fflush(stdout);
	
	now=q;
	print_node(now,1);
	if(now==NULL)return;
		
	while(now->next!=NULL)
	{
		printf("   | \n");
		printf("   | \n");
		now=now->next;
		print_node(now,1);
	}
	now=NULL;
	printf(RED"END OF QUEUE \n"NORMAL);
	fflush(stdout);
	return;
}

/*get i_size of head of queue*/
int get_isize(queue* q)
{
	int res=0;
	if(q==NULL) return -1;
	else
	{
		if(is_empty_q(q)) return 0;
		else
		{
			res=q->i_size;
			return res;
		}
	}
}

/*get a_size of head of queue*/
int get_asize(queue*q)
{
	int res=0;
	if(q==NULL) return -1;
	else
	{
		res=q->a_size;
		return res;
	}
}

/*return pointer of head->item*/
void* point_item(void *p, queue* q,int *myerror)
{
	if(p!=NULL || myerror==NULL)e_fatal("p !=NULL || myerror==NULL",0);

	memset(myerror,0x00,sizeof(int));

 	if(q==NULL)
	{
		*myerror=1;
		return NULL;
	}
	else
	{
		if(is_empty_q(q))
		{
			*myerror=2;
			return NULL;
		}
	}
	return q->item;
}

/*pop queue node but not free item*/
queue* pop_s_item(queue*q)
{
	queue* tmp=NULL;
	int check=0;

	if(q==NULL)e_fatal("queue is NULL",0);

	tmp=q;
	if(q->next!=NULL)
	{
		q=q->next;
		tmp->next=NULL;
		check=1;
	}
	tmp->item=NULL;
	memset(&(tmp->a_size),0x00,2);
	memset(&(tmp->i_size),0x00,2);
	if(check)/*i'm not the only*/
	{
		if(is_empty_q(tmp))
		{
			free(tmp);
			tmp=NULL;
		}
		else e_fatal("head of queue is not empty , can not do free",0);


	}
	return q;
}

/*this function is an exception rule of pure queue .
	add data to head of queue.
	Is use for data that have higth priority for Example
		Test data or */
queue* add_to_head(queue* q, void* item,const int size)
{
	queue* tmp=NULL;
	unsigned int a_size=0;

	if(size<=0 || size>2048) e_fatal("size must be > 0 or <2049",0);
	if(q==NULL)/*you have pass a null pointer or q point null*/
	{
		e_warning("NULL queue",0);
		return 0;
	}
	else
	{

		/*normalize a size*/
			if(size%2!=0)a_size=size+1;/*if is not multiple of 2*/
			else a_size=size;/*is multiple of 2*/

		/*end of normalization*/

		if(is_empty_q(q))/*queue is empty*/
		{
			q->item=malloc(a_size);/*allocate item in node*/
			if(q->item==NULL)e_fatal("malloc failed",0);
			memset(q->item,0x00,a_size);/*clear allocated memory*/

			memcpy(q->item,item,size);/*copy passed item in node*/
			q->a_size=a_size;
			q->i_size=size;
			q->next=NULL;/*set next to NULL*/
			return q;
		}
		else
		{
			tmp=new_queue(tmp);/*create new node*/

			tmp->item=malloc(a_size);/*allocate item in new node*/
			if(tmp->item==NULL)e_fatal("malloc failed",0);
			memset(tmp->item,0x00,a_size);/*clear allocated memory*/

			tmp->a_size=a_size;
			tmp->i_size=size;
			tmp->next=q;
			/*printf("settato \n");*/
			memcpy(tmp->item,item,q->i_size);/*copy item in node*/
			/*printf("esco!!! \n");*/
			return tmp;
		}
	}
}


int add_to_item(queue* q, void *data,int size)
{
	queue* tmp=NULL;
	byte *n=NULL;
	byte *p=NULL;
	short int dim;

	if(data==NULL || q==NULL)e_fatal("NULL pointer",0);

	if(size<=0)e_fatal("size must be >0",0);

	p=q->item;
	dim=q->i_size;
	q->i_size+=size;
	if((q->i_size)%2!=0)q->a_size=+q->i_size+1;
	else q->a_size=+q->i_size;

	q->item=NULL;
	q->item=malloc(q->a_size);
	if(q->item==NULL)
	{
#ifdef PROXY_DEBUG
		printf(RED"allocate item \n"NORMAL);
		fflush(stdout);
#endif
		q->item=p;
		p=NULL;
		return 0;
	}
	memcpy(q->item,p,dim);
	free(p);
	p=NULL;
	n=(byte*)q->item;
	n=&n[dim];
	memcpy(n,data,size);
	n=NULL;
	tmp=NULL;
#ifdef PROXY_DEBUG
	printf("I'm exit \n");
	fflush(stdout);
#endif
	return 1;

}

queue* delete_item(int size,queue* q)
{
	byte * mybyte=NULL;
	void *p=NULL;
	queue *head=NULL;
	
	if(q==NULL || size<0 || size>=q->i_size)
		e_fatal("queue must be !=NULL",0);
	
	p=malloc(q->i_size-size);
	if(p==NULL)
		e_fatal("malloc failed",0);
	
	mybyte=q->item;
	mybyte+=size;
	memcpy(p,mybyte,q->i_size-size);
	mybyte=NULL;
	head=new_queue(head);
	if(head==NULL)
		e_fatal("head is NULL",0);
	if(add_to_item(head,p,q->i_size-size))
	{
		q=pop_s_item(q);
		head->next=q;
		return head;
	}
	else return q;
}





#else
    #error ANSI C89 is not specified abort
#endif

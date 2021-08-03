
#include "l_list.h"

#ifndef L_LIST_H
#error no linked list header defined
#else

#ifndef L_LIST_C
#define L_LIST_C
#endif

#endif 

#if defined(PROXY_C89) && defined(L_LIST_C)
 
/*this function allocate new linked node and return address*/
l_list* new_linked(l_list *ls)
{
	if(ls!=NULL)e_fatal("ls must be NULL",0); 
	ls=new_queue(ls);
	if(ls==NULL)e_fatal("create list",0);
	return ls;
}

/*add data in linked
	use push() defined in queue
Complexity O(n) where n is length of list*/
int add_item(l_list *ls,void* data,int size)
{
	if(ls==NULL) e_fatal("ls must be !=NULL",0);
	if(data==NULL) return 0;
	
	if(push(data,ls,size)) return 1;
	else return 0;
}

/*add data to node at index index
Complexity O(n) where n is index passed to function*/
l_list* add_to(l_list *ls,void* data,int size,int index)
{
	l_list *prev=NULL;
	l_list *now=NULL;

	
	int i=0;
	
	if(ls==NULL|| data==NULL || index<0)e_fatal("bad args",0);
	if(index==0) /*is the first*/
	{
		now=new_linked(now);
		if(now==NULL)e_fatal("create linked",0);
		
		if(add_to_item(now,data,size))
		{
			now->next=NULL;
			now->next=ls;
			return now;
		}
		else return 0;
	}
	now=ls;	
	while(now!=NULL && i<index)
	{
		prev=now;/*i point new prev value*/
		now=now->next;/*i point new now value*/
	}
	if(i!=index)e_fatal("index is out of linked",0);/*index is out of linked*/
	
	prev->next=NULL;
	prev->next=new_linked(prev->next);/*create new node*/
	if(prev->next==NULL)e_fatal("can not create linked",0);
		
	if(add_item(prev->next,data,size)!=1)return 0;/*add data in new node*/
	(prev->next)->next=now;
	prev=NULL;
	now=NULL;
	
	return ls;
	
}

/*free() all node in list and return NULL*/
l_list* delete_linked(l_list *head)
{
	if(head==NULL)e_fatal("head must be !=NULL",0);
	head=delete_q(head);
	if(head!=NULL)e_warning("delete_linked() failed",0);
	return head;
}


/*get linked node at index index is auxiliary function*/
static l_list* get_link(int index,l_list *ls)
{
	if(ls->next==NULL && index>0) return NULL;
	if(index>0) return get_link(index-1,ls->next);
	
	return ls;
}

/*get linked node at index index*/
l_list* get_linked(int index,l_list* ls)
{
	if(ls==NULL) e_fatal("ls must be !=NULL",0);
	if(index <0)e_fatal("index must be >0",0);
	if(is_empty_q(ls)) return NULL;
	
	ls=get_link(index,ls);
	return ls;
}

/*this auxiliary function delete node at index index*/
/* is used when index is >=1*/
static l_list* delete_link_node(l_list *start,l_list *head,int index,int *res)
{
	if(head->next==NULL && index>1)/*failure*/
	{
		*res=1;
		return start;
	}
	if(index>1) delete_link_node(start,head->next,index-1,res);/*recursive*/
	if(index==1)
	{
		if(head->next!=NULL)
		{
			{
				l_list *tmp=NULL;
				l_list *next=NULL;
				
				tmp=head->next;
				next=tmp->next;
				tmp->next=NULL;
				
				tmp=delete_linked(tmp);
				/*
				if(tmp!=NULL)e_fatal("delete_link_node() failed",0);	*/
				
				head->next=next;
				next=NULL;
				return start;
			}
			
		}
		else
		{
			/*
			l_list *tmp=NULL;
			head=delete_linked(head);
			if(tmp!=NULL)e_fatal("delete_link_node() failed",0);*/
			return start;
		}
	}
	else return start;
}


l_list * delete_linked_node(l_list *head,int index)
{
	int res=0;
	l_list *next=NULL;
	if(head==NULL) e_fatal("head must be !=NULL",0);
		
	if(index<0)e_fatal("index must be >=0",0);
	
	if(index==0)/*index is 0*/
	{
		if(head->next==NULL)/*is the only node*/
		{	
			head=delete_linked(head);
			return head;
		}
		else/*is not the only node*/
		{
			next=head->next;
			head->next=NULL;
			head=delete_linked(head);
			return next;
		}
	}

	head=delete_link_node(head,head,index,&res);/*index is not 0*/
	/*if(res)e_fatal("delete_linked_node() failed",0);*//*check if function do his work*/
	memset(&res,0x00,sizeof(int));		
	return head;	
	
}


/*check if linked node is empty*/
int is_empty_linked(l_list *ls)
{
	if(is_empty_q(ls))return 1;
	else return 0;
}


/*delete all item  located from start to end(start and end passed value)*/ 
l_list* delete_to(int start,int end,l_list* head)
{
	int i=0;
	l_list *p=NULL;
	l_list *prev=NULL;
	
	if(start <0 || end <0 || end<start)e_fatal("one or two index are <0 or end is < start",0);
	
	if(head==NULL)e_fatal("head must be !=NULL",0);
	
	if(start>0)
	{
		prev=get_linked((start-1),head);
	
		if(prev!=NULL)
			p=prev->next;
		else 
			p=get_linked(start,head);
	}
	else
		p=get_linked(start,head);

	for(i=start;i<=end;i++)
	{
		p=delete_linked_node(p,0);
		if(p==NULL)break;
	}
	if(i!=end)
	{
		e_warning("end is a value out of list",0);
		printf(YELLOW"\t end of l_list is "RED"%d\n"NORMAL,i);
	}
	
	if(prev!=NULL)
		prev->next=p;
	
	p=NULL;
	prev=NULL;
	return head;
	
}

/*add one linked node at the end of list*/
int add_node(l_list *head, l_list *node)
{
	if(head==NULL) return 0;
		
	if(node==NULL) return 1;
		
	while(head->next!=NULL)
	{
		head=head->next;
	}
	head->next=node;
	return 1;
}

/*add one node at index index*/
l_list* add_node_index(l_list *head,l_list *node,int index)
{
	/*l_list * start=NULL;*/
	l_list *now=NULL;
	l_list *prev=NULL;
	int i=0;
	
	if(head==NULL) e_fatal("head must be !=NULL",0);
		
	if(node==NULL) return head;
	if(node->next!=NULL)e_fatal("pass a linked list not only node",0);
		
	if(index<0)e_fatal("index must be >0",0);
	if(index==0)
	{
		node->next=head;
		return node;
	}
		
	/*start=head;*/
	now=head;
	for(i=0;i<index;i++)
	{
		prev=now;
		now=now->next;
		if(now==NULL && i!=index-1)break;
	}
	if(i!=index)e_fatal("index is out of linked",0);
			
	prev->next=node;
	node->next=now;
	now=NULL;
	prev=NULL;
	return head;	
}
/*print linked list*/
void print_linked(l_list *head)
{
	l_list *now=NULL;

	printf(RED"START OF LINKED \n"NORMAL);
	fflush(stdout);
	
	now=head;
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
	printf(RED"END OF LINKED \n"NORMAL);
	fflush(stdout);
	return;
}



/*add data at node in index index*/
int add_data_linked(l_list *head,int index,void* data,int size)
{
	l_list *now=NULL;
	int i=0;
	int res=0;
	
	if(index<0)e_fatal("index must be >0",0);
		
	if(index==0)
	{
		if(add_to_item(head,data,size)) return 1;
		else return 0;
	}
	else
	{
		now=head;
		for(i=0;((i<index));i++)
		{
			now=now->next;
			if(now==NULL && i!=index-1)break;
		}			
		if(i!=index)e_fatal("index is out of list",0);
		
		res=add_to_item(now,data,size);
		now=NULL;
		if(res) return 1;
		else return 0;
	}
}


l_list* go_next(l_list *ls)
{
	if(ls==NULL)e_fatal("ls must be !=NULL",0);
	return ls->next;
}

#else

#error No l_list.c file or compiler is not conform to C89

#endif

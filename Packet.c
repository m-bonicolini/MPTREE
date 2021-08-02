#include <arpa/inet.h>
#include "Packet.h"

#ifndef PACKET_H
	#error Packet.h header not found
#else	
	#ifndef PACKET_C
	#define PACKET_C
	#endif
#endif


	

/*create new clear packet*/
packet* new_packet(packet*p)
{
	if(p!=NULL)
		e_fatal("packet must point NULL",0);
	
	
#if defined(PROXY_LP32) || defined(PROXY_ILP32)
	p=(packet*) malloc(12);
#else
	#if defined(PROXY_LP64) || defined(PROXY_ILP64)
		p=(packet*)malloc(18);
	#else
		#error unknow arch
	#endif
#endif
	
	if(p==NULL)
		e_fatal("malloc failed",errno);
	
	memset(p,0x00,6);
	p->data=NULL;
	return p;
}


packet* make_packet(short int type,short int id,short int len,void* data)
{
	packet *p=NULL;

	
	p=(packet*) malloc(sizeof(packet));

#ifdef PROXY_BIG
	p->type=type;
	p->serial=id;
	p->len=len;
#else
	p->type=htons(type);
	p->serial=htons(id);
	p->len=htons(len);
#endif
	p->data=NULL;
	
	if(data==NULL && (type==ACK || type==QUIT))
	return p;
	else
	{
		p->data=malloc(len-6);
		memcpy(p->data,data,len-6);
		return p;
	}
}

void* delete_packet(packet *p)
{
	if(p==NULL)
		e_fatal("delete packet",0);
	
	if(p->data!=NULL)
	{
		free(p->data);
		p->data=NULL;
	}
	free(p);
	p=NULL;
	return p;
}


 
packet* parse_packet(void *buffer ,int n_read)
{
	packet *p=NULL;
	byte *index=NULL;
	
	if(buffer==NULL)
		e_fatal("buffer must be != NULL",0);
	
	p=new_packet(p);
	if(p==NULL)
		e_fatal("new_packet() fail",0);
	
	if(n_read<6)
		return NULL;
	
	index=buffer;
	memcpy(&p->type,&index[0],2);
	memcpy(&p->serial,&index[2],2);
	memcpy(&p->len,&index[4],2);
	
	
#ifndef PROXY_BIG
	p->type=ntohs(p->type);
	p->serial=ntohs(p->serial);
	p->len=ntohs(p->len);
#endif
	if(p->len<6 || p->len >1024)
		return p;
	
	if(n_read < p->len)
	{
		p=delete_packet(p);
		if(p!=NULL)
			e_fatal("delete packet failed",0);
		return p;
	}
	
	p->data=malloc(p->len-6);
	if(p->data==NULL)
		e_fatal("malloc failed",errno);
	memset(p->data,0x00,p->len-6);
	
	memcpy(p->data,&index[6],p->len-6);
	return p;
}

int check_packet(packet *p)
{
	int types[4];
	int i=0;
	int flag=0;

	types[0]=DATA;
	types[1]=QUIT;
	types[2]=DIE;
	types[3]=ACK;
	
	if(p==NULL)
		return 0;
	for(i=0;i<4;i++)
	{
		if(p->type==types[i])
		{
			flag=1;
			break;
		}
	}
	if(!flag)
		return 0;

	if(p->serial <0)
		return 0;
	
	if(p->len>1024 || p->len<6)
		return 0;
	
	if(p->data==NULL && p->type!=ACK)
		return 0;
	
	switch(p->type)
	{
		/*case TEST:
		{
			if(p->len!=1024)
				flag=0;
		}break;
		case TEST_DATA:
		{
			if(p->len<7)
				flag=0;
		}break;*/
		case ACK:
		{
			if(p->len!=6)
				flag=0;
		}break;
	}
	return flag;
}

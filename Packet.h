
#include "Message.h"


#ifdef PROXY_C89
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else 
	#error compiler does not support C89
#endif



#ifndef PACKET_H
#define PACKET_H
#endif 


#define DATA 	1
#define QUIT 	2
#define DIE 	3 /* not used yet*/
#define ACK 	4




struct pack
{
	/*-----------------header-------------------------*/
	short int type;
	short int serial; 
	short int len;	/*header+data*/
	/*----------------end header-------------------*/
	
	void *data;
};

typedef struct pack packet;
	

extern packet* new_packet(packet*p);/*make new empty packet*/
extern packet* make_packet(short int type,short int id,short int len,void* data);/*make new packet with 
																												filled header*/
extern void* delete_packet(packet *p);
extern packet* parse_packet(void *buffer ,int n_read);/*obsolete*/

#include <stdlib.h>
#include <string.h> 
#include <sys/time.h>
#include "Message.h"


#ifndef CHANNEL_H
#define CHANNEL_H
#endif 

#ifndef CHANNEL_H
#error No header found in Channel.h
#endif

#if (defined(PROXY_ILP32) || defined(PROXY_LP64) || defined(PROXY_ILP64)) || defined(PROXY_LP32)

struct chan 
{
	int listenfd;				/*listenfd of channel*/
	int fd; 						/*fd of channel*/
	void *pak;	               /*send queue of channel*/
	short int port;
 #ifdef PROXY_LP32	
	long int available;	/*status of channel*/
	long int ack_lock;		/*flag for read_ack() permission */
	long int success;		/*write permission*/		
	long int score;	   /*channel MTU*/
	int losed;
	unsigned long int ip; /*obsolete*/
 #else	/*if it is ILP64  int is 64 bits*/
	int available;	
	int ack_lock;		
	int success;				
	int score;			
	int losed;
	unsigned int ip;
	/*SO_IN addr;*/
 #endif
	
};
#else 
  #error error unkonow arch

#endif


typedef struct chan channel;

#ifdef PROXY_C89

extern channel* new_channel(channel *c);
extern int get_listenfd(channel *c);
extern int get_fd(channel *c);

#if !defined(PROXY_LP32)
extern int get_avaiable(channel *c);
#else 
extern long get_avaiable(channel *c);
#endif
#if !defined(PROXY_LP32)
extern int get_success(channel *c);
#else
extern long get_success(channel *c);
#endif
#if !defined(PROXY_LP32)
extern int get_score(channel *c);
#else
extern long get_score(channel *c);
#endif

extern void set_listenfd(channel *c,int value);
extern void set_fd(channel *c,int value);

#if !defined(PROXY_LP32)
extern void set_available(channel *c,int value);
#else 
extern void set_available(channel *c,long value);
#endif 
/*
#if !defined(PROXY_LP32)
extern void set_failure(channel *c,int value);
#else 
extern void set_failure(channel *c,long value);
#endif*/ 
#if !defined(PROXY_LP32)
extern void set_success(channel *c,int value);
#else 
extern void set_success(channel *c,long value);
#endif 
#if !defined(PROXY_LP32)
extern void set_score(channel *c,int value);
#else 
extern void set_score(channel *c,long value);
#endif 
extern int set_wrost(channel *c,struct timeval value);/*obsolete*/
extern int set_best(channel *c,struct timeval value);/*obsolete*/
extern int set_expire(channel *c,struct timeval value);/*obsolete*/
extern void print_channel(channel *c);
extern void print_channel_q(channel *c);
#endif

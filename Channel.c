#include "Channel.h"

#ifndef CHANNEL_H
#error no linked list header defined
#else

#ifndef CHANNEL_C
#define CHANNEL_C
#endif

#endif 	

#if defined(PROXY_C89) && defined(CHANNEL_C)

channel* new_channel(channel *c)
{
	if(c!=NULL)e_fatal("pass an assigned pointer",0);
		
	c=(channel*) malloc(sizeof(channel));
	if(c==NULL)e_fatal("malloc failed",0);
	
#ifdef PROXY_ILP64
	memset(&(c->listenfd),0x00,8);
	memset(&(c->fd),0x00,8);
	memset(&(c->available),0x00,8);
	memset(&(c->ack_lock),0x00,8);
	memset(&(c->success),0x00,8);
	memset(&(c->score),0x00,8);
#else
	memset(&(c->listenfd),0x00,4);
	memset(&(c->fd),0x00,4);
	memset(&(c->available),0x00,4);
	memset(&(c->ack_lock),0x00,4);
	memset(&(c->success),0x00,4);
	memset(&(c->score),0x00,4);
#endif
	c->pak=NULL;
	return c;
}


int get_listenfd(channel *c)
{
	if(c==NULL) e_fatal("pass a channel",0);
		
	return c->listenfd;
}

int get_fd(channel *c)
{
	if(c==NULL)e_fatal("pass a channel",0);
		
	return c->fd;
}


#if !defined(PROXY_LP32)
int get_avaiable(channel *c)
#else 
long get_avaiable(channel *c)
#endif
{
	if(c==NULL)e_fatal("pass a channel",0);
		
	return c->available;
}



#if !defined(PROXY_LP32)
int get_success(channel *c)
#else
long get_success(channel *c)
#endif
{
	if(c==NULL)e_fatal("pass a channel",0);
		
	return c->success;
}

#if !defined(PROXY_LP32)
int get_score(channel *c)
#else
long get_score(channel *c)
#endif
{
	if(c==NULL)e_fatal("you must pass a channel",0);
		
	return c->score;
}

void set_listenfd(channel *c,int value)
{
	if(c==NULL) e_fatal("you must pass a channel",0);
	if(value<0) e_fatal("you must pass a value >0",0);
	c->listenfd=value;
}

void set_sfd(channel *c,int value)
{
	if(c==NULL) e_fatal("you must pass a channel",0);
	if(value<0) e_fatal("you must pass a value >0",0);
	c->fd=value;
}
#if !defined(PROXY_LP32)
void set_available(channel *c,int value)
#else 
void set_available(channel *c,long value)
#endif 
{
	if(c==NULL) e_fatal("you must pass a channel",0);
	if(value<0) e_fatal("set_available() : wrong available value given as input",0);
	c->available=value;
}


#if !defined(PROXY_LP32)
void set_success(channel *c,int value)
#else 
void set_success(channel *c,long value)
#endif 
{
	if(c==NULL) e_fatal("you must pass a channel",0);
	if(value<0) e_fatal("set_success() : wrong success value given as input",0);
	c->success=value;
}

#if !defined(PROXY_LP32)
void set_score(channel *c,int value)
#else 
void set_score(channel *c,long value)
#endif 
{
	if(c==NULL) e_fatal("you must pass a channel",0);
	if(value<0) e_fatal("set_score() : wrong score value given as input",0);
	c->score=value;
}



static void print_chan(channel *c,int send)
{
	if(c==NULL)e_fatal("channel must be !=NULL",0);
	if(send<0 || send >1)e_fatal("channel flag must be 1 or 0",0);
	
	printf(RED" I print Channel stats \n"NORMAL);
	fflush(stdout);
#if defined(PROXY_LP32) 
		
		printf(GREEN" listenfd= %d\n"NORMAL,c->r_fd);
		fflush(stdout);
		printf(GREEN" fd= %d\n"NORMAL,c->s_fd);
		fflush(stdout);
		printf(GREEN" available= %ld\n"NORMAL,c->available);
		fflush(stdout);
		printf(GREEN" ack_lock= %ld\n"NORMAL,c->ack_lock);
		fflush(stdout);
		printf(GREEN" success= %ld\n"NORMAL,c->success);
		fflush(stdout);
		printf(GREEN" score= %ld\n"NORMAL,c->score);
		fflush(stdout);
#else
		printf(GREEN" listenfd= %d\n"NORMAL,c->listenfd);
		fflush(stdout);
		printf(GREEN" fd= %d\n"NORMAL,c->fd);
		fflush(stdout);

		printf(GREEN" available= %d\n"NORMAL,c->available);
		fflush(stdout);
		printf(GREEN" failure= %d\n"NORMAL,c->ack_lock);
		fflush(stdout);
		printf(GREEN" success= %d\n"NORMAL,c->success);
		fflush(stdout);
		printf(GREEN" score= %d\n"NORMAL,c->score);
		fflush(stdout);
#endif	
		
		printf(RED" end of CHANNEL \n"NORMAL);
		fflush(stdout);
	return ;
}



void print_channel(channel *c)
{
	print_chan(c,0);
	return;
}

void print_channel_q(channel *c)
{
	print_chan(c,1);
	return;
}
#else

#error NO C89 compiler

#endif

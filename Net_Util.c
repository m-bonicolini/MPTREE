
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>


#include "Net_Util.h"


struct timeval differenza(struct timeval dopo,struct timeval prima)
{
	struct timeval diff;
	normalizza( &prima);
	normalizza( &dopo);
	
	diff.tv_sec = dopo.tv_sec - prima.tv_sec;          /* Sottraggo i secondi tra di loro */
	if (diff.tv_sec < 0){
		diff.tv_sec = 0;
		diff.tv_usec = 0;
	}
	else{
		diff.tv_usec = dopo.tv_usec - prima.tv_usec;
		if  (diff.tv_usec < 0){
			if (diff.tv_sec > 0)
			{
				/*
				 Devo scalare di uno i secondi e sottrarli ai micro secondi 
				 ossia aggiungo 1000000 all'ultima espressione
				 */
				diff.tv_sec =diff.tv_sec - 1;
				diff.tv_usec =(dopo.tv_usec) - prima.tv_usec + SEC_IN_MCSEC;
			}
			else{
				diff.tv_usec = 0;
			}
		}
	}
	return(diff);
}

/*Check if value is 1 or 0*
return 0 if  value <0 or value> 1*/
static int check_binary(int value)
{
	if(value==0 || value==1)
	{
		return 1;/*is bool value*/
	}
	else
	{
		return 0;
	}
}

/*check if "option" is a socket option.
Is auxiliary function of proxy_sock_opt()
(not all socket option are supported.).*/
static int is_sockop(int s_opt)
{
#ifdef PROXY_LINUX
	if(s_opt==SO_RCVBUFFORCE ||
		s_opt==SO_RCVLOWAT      ||
		s_opt==SO_PEERCRED
	)return 1;
#endif 
	if(s_opt==SO_BROADCAST    	||
		s_opt==SO_DEBUG 			  	||
		s_opt==SO_DONTROUTE   	||
		s_opt==SO_KEEPALIVE     	||
		s_opt==SO_LINGER			  	||
		s_opt==SO_ACCEPTCONN	||
		s_opt==SO_OOBINLINE	   	||
		s_opt==SO_REUSEADDR )
	return 1;/*binary opt*/
	if(s_opt==SO_SNDBUF    		|| 
		s_opt==SO_RCVBUF 		  	|| 
		s_opt==SO_SNDLOWAT  		|| 
		s_opt==SO_RCVLOWAT)
	return 2;/*non binary opt*/
	else return 0;/*error invalid option or 
		option is not supported by proxy_setsock()*/
}



/*check if option is a tcp_option (IP_PROTO...)
  Is auxiliary function of proxy_sock_opt*/
static int is_tcp_opt(int t_opt)
{
	#ifdef PROXY_LINUX
	if(t_opt==TCP_CORK    					||
		t_opt==TCP_DEFER_ACCEPT		||
		t_opt==TCP_INFO						||
		t_opt==TCP_QUICKACK				||
		t_opt==TCP_WINDOW_CLAMP)
	return 1;/*Binary Option*/
	if(	t_opt==TCP_KEEPCNT				||
			t_opt==TCP_KEEPIDLE				||
			t_opt==TCP_KEEPINTVL			||	
			t_opt==TCP_LINGER2					)
	return 2;/*non binary opt*/
	#endif
	
	if(t_opt==TCP_NODELAY) return 1;
	
	if(	t_opt==TCP_MAXSEG ) return 2;
	
	return 0;
}



int proxy_sock_opt(int socket ,int socket_op,int value,int* myerror)
{
	int opt_val=value;
	int check=0;
	
	/*value fro sock option is <0 , error*/
	if(value<0)
	{
		e_warning("Value must be >=0",0);
		return 0;/*error*/
	}
	
	
	check=is_sockop(socket_op);
	if(check>0)/*if is socket option*/
	{
		/*u have pass a non binary value 
		  in a binary option socket 						*/
		if(check_binary(value)==0 && check==1)
		{
			e_warning("You must pass 1 or 0 value",0);
			return 0;/*error*/
		}
		else
		{
			if(socket_op==SO_LINGER)
				check= setsockopt(socket, SOL_SOCKET, socket_op, (char *)&opt_val, sizeof(struct linger));
			else
				check= setsockopt(socket, SOL_SOCKET, socket_op, (char *)&opt_val, sizeof(int));
			*myerror=errno;
			if(check!=0)
			{
				if(*myerror==EFAULT || *myerror==EINVAL)
				e_fatal("proxy_sockopt()",*myerror);
				else return 0;
			}
			else return 1;
		}
	}
	else
	{
		e_warning("You pass a wrong option or option is not supported by function",0);
		return 0;
	}
	return 0;
}

int proxy_tcp_opt(int socket,int socket_op,int value,int *myerror)
{
	int opt_val=value;
	int check=0;
	if(value<0)
	{
		e_warning("Value must be >=0",0);
		return 0;
	}
	
	check=is_tcp_opt(socket_op);
	if(check>0 )
	{
		if(check_binary(value)==0 && check==1)/*value is not binary and but option request binary value*/
		{
			e_warning("You must pass 1 or 0 value",0);
			return 0;
		}
		else
		{
			check= setsockopt(socket, IPPROTO_TCP, socket_op, (char *)&opt_val, sizeof(opt_val));
			*myerror=errno;
			if(check!=0)
			{
				if(*myerror==EFAULT || *myerror==EINVAL)
				e_fatal("proxy_sockopt()",*myerror);
				else return 0;
			}
			else return 1;
		}
	}
	else return 0;
		
	return 0;
}

/*This function is for listening.
  args are:
	listenfd is pointer where store socket; 
	string_IP is pointer to ip in string apparence; 
	send buffer and recive buffer size are s_buf and r_buf;
	tcpnodelay is flag for tcpnodelay option(boolean 1 or 0);
	myerror is pointer where store error after call any function()*/
static int p_listening(int *listenfd, char *string_IP, int port,const int s_buf,const int r_buf,int tcpnodelay,int* myerror)
{
	SO_IN local;
	int check=0;
	int dim=0;
	unsigned int len=sizeof(int);
	
#if defined(PROXY_DEBUG) || defined(PEDANTIC)|| defined(DEBUG) 
	if(listenfd==NULL) 
	{
		e_warning("listenfd must be !=NULL proxy_listening()",0); 
		return 0;
	}
	if(string_IP==NULL)
	{
		e_warning("IP must be !=NULL proxy_listening()",0);
		return 0;
	}
	if(myerror==NULL)
	{		
		e_warning(" error pointer must be !=NULL",0);
		return 0;
	}
	if(s_buf<=0) 
	{
		e_warning("invalid value!! s_buf must be >0 proxy_listening()",0);
		return 0;
	}
	if(r_buf<=0 ) 
	{
		e_warning("invalud value!! r_buf must be >0  proxy_listening()",0);
		return 0;
	}
#else
	if(listenfd==NULL||myerror==NULL||string_IP==NULL)
	{
		e_warning("You have pass a null in one (or more) pointer of proxy_listening()",0);
		/*
		#if defined(PROXY_DEBUG_LOG)
		log=fopen("proxyS.log",a);
		fputs("\n [debug p_listening] bad args \n");
		fclose(log);
		#endif*/
		
		return 0;		
	}
	if(s_buf<=0 || r_buf<=0)
	{
		e_warning("you have pass a 0 or <0 number in one(or more) buffer option",0);
		/*
		#if defined(PROXY_DEBUG_LOG)
		log=fopen("proxyS.log",a);
		fputs("\n [debug p_listening] bad args \n");
		fclose(log);
		#endif*/
		
		return 0;
	}
#endif
	
	*listenfd=socket(AF_INET,SOCK_STREAM,0);
	*myerror=errno;
	if(*listenfd==-1)	
	{
		if(*myerror==EMFILE) e_fatal("Proxy_listening()",*myerror);
		else 
		{
			/*#if defined(PROXY_DEBUG_LOG)
			log=fopen("proxyS.log",a);
			fputs("\n [debug p_listening] \n");
			fclose(log);
			#endif*/
			
			return 0;
		}
	}
/*
#if defined(PROXY_DEBUG_LOG)
	log=fopen("proxyS.log",a);
	fputs("[debug p_listening] listenfd after socket()=%d",*listenfd);
	fclose(log);
#endif*/
 

#ifdef PROXY_DEBUG
	printf(RED"After socket() listenfd=%d\n"NORMAL,*listenfd);
#endif
	
	check =proxy_sock_opt(*listenfd,SO_REUSEADDR,1,myerror);
	if(check==0)
	{
#ifdef PROXY_DEBUG
		e_warning("SO_REUSEADDR proxy_listening()",*myerror);
#endif
		return 0;
	}
	
#ifdef PROXY_DEBUG
	e_warning("i'm going to set option of socket",0);
#endif
	
	check=proxy_sock_opt(*listenfd,SO_SNDBUF,s_buf,myerror);
	if(check==0)
	{
		e_fatal(NULL,*myerror);
		return 0;/*paranoid*/
	}
	else
	{
#ifdef PROXY_DEBUG
		printf(GREEN"I had set option of socket\n"NORMAL);
		fflush(stdout);
#endif

		check=getsockopt(*listenfd,SOL_SOCKET,SO_SNDBUF,(char*)&dim,&len);
		*myerror=errno;
		if(check!=0)/*getsockopt return error*/
		{
			e_warning("get sock opt failed",0);
			/*fatal error ,exit  now!*/
			if(*myerror==EFAULT ||*myerror==EINVAL)
			e_fatal(NULL,0);
			else return 0;/*error is not fatal. 
				listen but i don't quit from main program*/
		}
		else
		{
#if !defined(PROXY_OSX)  && !defined(PROXY_BSD)
			if(dim!=(s_buf*2)) return 0;
#endif
		}
#ifdef PROXY_DEBUG
		msg_ok("SNDBUF is set");	    
		printf(RED"[debug] Snd_Buffer size=%d\n"NORMAL,dim);
		fflush(stdout);
#endif
	}
	
#ifdef PROXY_DEBUG
	e_warning("I'm going to set sock option",0);
#endif
	
	check=proxy_sock_opt(*listenfd,SO_RCVBUF,r_buf,myerror);
	if(check==0)	e_fatal(NULL,*myerror);
	else
	{
#ifdef PROXY_DEBUG
		printf(GREEN"I had set option of socket\n"NORMAL);
		fflush(stdout);
#endif
		check=getsockopt(*listenfd,SOL_SOCKET,SO_RCVBUF,(char*)&dim,&len);
		*myerror=errno;
		if(check!=0)
		{
			e_warning("get sock opt failed",0);
			if(*myerror==EFAULT ||*myerror==EINVAL)
			e_fatal(NULL,0);/*fuction print message and exit(0)*/
			else return 0;
		}
#ifdef PROXY_DEBUG
		msg_ok("RCVBUF is set");	    
		printf(RED"[debug] RCV_Buffer size=%d\n"NORMAL,dim);
#endif
	}
	if(tcpnodelay==1)
	{
#ifdef PROXY_DEBUG
		e_warning("I'm going to set tcp_nodelay",0);
#endif
		check=proxy_tcp_opt(*listenfd,TCP_NODELAY,1,myerror);
		*myerror=errno;
		if(check==0)	
		{
			#if defined(PROXY_DEBUG) || defined(DEBUG)
			printf(RED"[debug] I can not set tcpnodel on p_listening()"NORMAL);
			fflush(stdout);
			#endif
			return 0;
		}
		else
		{
			check=getsockopt(*listenfd,IPPROTO_TCP,TCP_NODELAY,(char*)&dim,&len);
			*myerror=errno;
			if(check!=0)
			{
				if(*myerror==EFAULT ||*myerror==EINVAL)
				e_fatal(NULL,0);
				else 
				{
					e_warning(NULL,*myerror);
					return 0;
				}/*cotrollare che tcp no delay sia attivato dopo get*/
			}
			msg_ok("TCP_NODELAY");	    
		}
	}
	
	memset ( &local, 0x00, sizeof(local) );
	
	local.sin_family=AF_INET;
	check=inet_aton(string_IP,&(local.sin_addr));
	if(check==0)
	{
		e_warning("invalid ip address",0);
		return 0;
	}
#if ! defined(PROXY_BIG)
	local.sin_port=htons(port);	
#else
	local.sin_port=port;
#endif
	check=bind(*listenfd,(SO_A*)&local,sizeof(local));
	*myerror=errno;
	if(check<0)
	{
		if(*myerror==EFAULT || *myerror==EINVAL || *myerror==ELOOP)
		e_fatal(NULL,*myerror);
		else return 0;
	}
#ifdef PROXY_BSD
	check=listen(*listenfd,6);
#else
	check = listen(*listenfd, 9);
#endif
	if(check<0) return 0;
	msg_ok("Listening");
	printf(GREEN"I'm listen on port %d and ip %s \n"NORMAL,port,string_IP);
	return 1;
}

int proxy_listening(char *remote_IP,int port)
{
	int fd=0;
	int myerror=0;
	int res=0;
	
	if(remote_IP==NULL)e_fatal("you must pass IP Number",0);
	if(port<=0) e_fatal("port must be >0",0);
	res=p_listening(&fd,remote_IP,port,3000,3000,1,&myerror);
	if(res<=0 || myerror>0)e_fatal("Listening failed",myerror);
		
	if(fd>0) return fd;
	else      return -1;
}

int advanced_listening(char* remote_IP,int port,int s_buff,int r_buff)
{
	int fd=0;
	int myerror=0;
	int res=0;
	int p_sbuff=0;
	int p_rbuff=0;
	
	if(remote_IP==NULL)e_fatal("you must pass IP Number",0);
	if(port<=0) e_fatal("port must be >0",0);
	if(s_buff<=0 || r_buff<=0)e_fatal("bad buffer value",0);
	
	p_sbuff=s_buff/2;
	p_rbuff=r_buff/2;
	
	if(p_sbuff<1)p_sbuff=1;
	if(p_rbuff<1)p_rbuff=1;

	res=p_listening(&fd,remote_IP,port,p_sbuff,p_rbuff,1,&myerror);
	if(res<0 || myerror>0)e_fatal("Listening failed",myerror);
		
	if(fd>0) return fd;
	else      return -1;
}

static int p_connect(int* serverfd,char* remote_IP,int port,int s_buf,int r_buf,int tcpnodelay,int* myerror)
{
	SO_IN local,remote;
	int check=0;
	int dim=0;
	unsigned int len=sizeof(int);
	
	if(serverfd==NULL || remote_IP==NULL || s_buf <=0 || r_buf <=0)
	return 0;
		
	
	*serverfd=socket(AF_INET,SOCK_STREAM,0);
	*myerror=errno;
	if(*serverfd==-1)
	{
		if(*myerror==EMFILE) e_fatal("socket()",*myerror);
		else return 0;
	}
	check =proxy_sock_opt(*serverfd,SO_REUSEADDR,1,myerror);
	if(check==0)
	{
		e_warning(NULL,*myerror);
		return 0;
	}
	check=proxy_sock_opt(*serverfd,SO_SNDBUF,s_buf,myerror);
	if(check==0) e_fatal(NULL,*myerror);
	else
	{
		check=getsockopt(*serverfd,SOL_SOCKET,SO_SNDBUF,(char*)&dim,&len);
		*myerror=errno;
		if(check!=0)
		{
			if(*myerror==EFAULT ||*myerror==EINVAL)  e_fatal(NULL,0);
			return 0;
		}
#ifdef PROXY_DEBUG
		msg_ok("SNDBUF is set");	    
		printf("[debug] Snd_Buffer size=%d\n",dim);
#endif
	}
	
	check=proxy_sock_opt(*serverfd,SO_RCVBUF,r_buf,myerror);
	if(check==0)
	{
		e_fatal(NULL,*myerror);
		return 0;/*paranoid*/
	}
	else
	{
		check=getsockopt(*serverfd,SOL_SOCKET,SO_RCVBUF,(char*)&dim,&len);
		*myerror=errno;
		if(check!=0)
		{
			if(*myerror==EFAULT ||*myerror==EINVAL) e_fatal(NULL,0);
			else 
			{
				e_warning(NULL,*myerror);
				return 0;
			}
		}
		/*else
		{
			if(dim!=r_buf*2)
			{
				printf("la sizeof buffer is not the specified\n");
				return 0;
			}
		}*/
#ifdef PROXY_DEBUG
		msg_ok("RCVBUF is set");	    
		printf("[debug] RCV_Buffer size=%d\n",dim);
#endif
	}
	
	if(tcpnodelay==1)
	{
		check=proxy_tcp_opt(*serverfd,TCP_NODELAY,1,myerror);
		*myerror=errno;
		if(check==0)	
		{
			#ifdef PROXY_DEBUG
			printf(RED"[debug] I can not set tcpnodel on p_listening()"NORMAL);
			#endif
			return 0;
		}
		else
		{
			check=getsockopt(*serverfd,IPPROTO_TCP,TCP_NODELAY,(char*)&dim,&len);
			*myerror=errno;
			if(check!=0)
			{
				if(*myerror==EFAULT ||*myerror==EINVAL)
				e_fatal(NULL,0);
				else 
				{
					e_warning(NULL,*myerror);
					return 0;
				}
			}
			msg_ok("TCP_NODELAY");	    
		}
	}
	

	
	memset (&local, 0x00, sizeof(local));
	local.sin_family=AF_INET;
	#ifdef PROXY_BIG
	local.sin_addr.s_addr=INADDR_ANY; 
	local.sin_port=0;
	#else
	local.sin_addr.s_addr=htonl(INADDR_ANY);
	local.sin_port=htons(0);
	#endif
	check=bind(*serverfd,(SO_A*)&local,sizeof(local));
	*myerror=errno;
	if(check<0) return 0;
	
	memset ( &remote, 0x00, sizeof(remote));
	remote.sin_family=AF_INET;
	check=inet_aton(remote_IP,&(remote.sin_addr));
	*myerror=errno;
	if(check==0)
	{
		e_warning("Error bad address",*myerror);
		return 0;
	}
#ifdef PROXY_BIG
	remote.sin_port=port;
#else
	remote.sin_port=htons(port);
#endif
	
	printf(GREEN"I'm trying to connect to %s\n"NORMAL,remote_IP);
	fflush(stdout);
	do
	{
		check=connect(*serverfd,(SO_A*)&remote,sizeof(remote));
	}while((check<0)&& (errno==EINTR));
	*myerror=errno;
	if(check<0)
	{
		if(*myerror==EFAULT) e_fatal("connect()",*myerror);
		else return 0;			
	}
	printf (GREEN"connected to %s %d\n"NORMAL, remote_IP, port);
	fflush(stdout);
	return 1;
}

int proxy_connect(char *remote_IP,int port)
{
	int fd=0;
	int myerror=0;
	int res=0;
	
	if(remote_IP==NULL)e_fatal("you must pass IP Number",0);
	if(port<=0) e_fatal("port must be >0",0);
	res=p_connect(&fd,remote_IP,port,3000,3000,1,&myerror);
	if(res<0 || myerror>0)e_fatal("connect failed",myerror);
		
	if(fd>0) return fd;
	else      return -1;
}

int advanced_connect(char* remote_IP,int port,int s_buff,int r_buff)
{
	int fd=0;
	int myerror=0;
	int res=0;
	int p_sbuff=0;
	int p_rbuff=0;
	
	if(remote_IP==NULL)e_fatal("you must pass IP Number",0);
	if(port<=0) e_fatal("port must be >0",0);
	if(s_buff<=0 || r_buff<=0)e_fatal("bad buffer value",0);
	
	p_sbuff=s_buff/2;
	p_rbuff=r_buff/2;
	
	if(p_sbuff<1)p_sbuff=1;
	if(p_rbuff<1)p_rbuff=1;

	res=p_connect(&fd,remote_IP,port,p_sbuff,p_rbuff,1,&myerror);
	if(res<0 || myerror>0)e_fatal("Listening failed",myerror);
		
	if(fd>0) return fd;
	else      return -1;
}

int set_tcpnodelay(int fd)
{
	int check=0;
	int myerror=0;
	int opt=0;
	socklen_t len=0;
	len=sizeof(int);
	
	if(fd<0)e_fatal("fd must be >0",0);
	else
	{
		/*check if option is just enable*/
		check=getsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char*)&opt,&len);
		myerror=errno;
		if(check!=0)
		{
			if(myerror==EFAULT || myerror==EINVAL)
			e_fatal(NULL,0);
			else 
			{
				e_warning(NULL,myerror);
				return 0;
			}
		}
		if(opt)/*option is just enable*/
		{
			e_warning("TCPNODELAY option is just enable",0);
			return 2;
		}
		/*option is not enable*/
		check=proxy_tcp_opt(fd,TCP_NODELAY,1,&myerror);
		myerror=errno;
		if(check==0 || myerror>0)	
		{
			#ifdef PROXY_DEBUG
			e_warning("i can not set tcpnodelay option",myerror);
			#endif
			return 0;
		}
		/*check if option is enable*/
		check=getsockopt(fd,IPPROTO_TCP,TCP_NODELAY,(char*)&opt,&len);
		myerror=errno;
		if(check!=0)
		{
			if(myerror==EFAULT ||myerror==EINVAL)
			e_fatal(NULL,0);
			else 
			{
				e_warning(NULL,myerror);
				return 0;
			}
		}
	}
	if(opt) return 1;
	else return 0;
}

int disable_tcpnodelay(int fd)
{
	int check=0;
	int myerror=0;
	
	if(fd<0)e_fatal("fd must be >0",0);
	else
	{
		check=proxy_tcp_opt(fd,TCP_NODELAY,0,&myerror);
		myerror=errno;
		if(check==0 || myerror>0)	
		{
			#ifdef PROXY_DEBUG
			e_warning("i can not disable tcpnodelay option",myerror);
			#endif
			return 0;
		}
	}
	return 1;
}


int normalizza( struct timeval *t )
{
	if(t->tv_usec>=SEC_IN_MCSEC)
	{
		t->tv_sec  += ( t->tv_usec/SEC_IN_MCSEC );
		t->tv_usec =  ( t->tv_usec%SEC_IN_MCSEC );
	}
	return(1);
}

int somma(struct timeval tmr,struct timeval ist,struct timeval *delay)
{
	normalizza( &tmr );
	normalizza( &ist );
	
	(*delay).tv_sec = ist.tv_sec + tmr.tv_sec;
	(*delay).tv_usec = ist.tv_usec + tmr.tv_usec;
	normalizza( delay );
	return(1);
}


int minore(struct timeval *a, struct timeval *b)
{
	normalizza(a);
	normalizza(b);
	if(a->tv_sec<b->tv_sec) return(1);
	else if (a->tv_usec<b->tv_usec) return(1);
	else return(0);
}


void stampa_timeval(char *str, struct timeval tv)
{
	printf("%s %ld.%3ld sec\n", str, tv.tv_sec, (long int)tv.tv_usec);
	fflush(stdout);
}


int send_packet(int fd,packet *p)
{
	void *buffer=NULL;
	byte *b=NULL;
	int nwrite=0;
	unsigned short int tot=0;
	int myerror=0;
	unsigned short len=0;
	fd_set fset;

	
	
	if(p==NULL)
		e_fatal("you have pass a NULL packet",0);
	
	if(ntohs(p->len)>=6 && ntohs(p->len)<=1024)
	{
		len=ntohs((short)p->len);
#ifdef PROXY_DEBUG
		printf("malloc di %d\n",len);
		fflush(stdout);
#endif		
		buffer=(void*)malloc(len);
		if(buffer==NULL)
			e_fatal("malloc failed",errno);
	}
	else
		e_fatal("packet len <= or > 1024 packet is malformed",0);
	
	b=(byte*)buffer;

/*	
#ifndef PROXY_BIG
	p->type=ntohs(p->type);
	p->serial=ntohs(p->serial);
	p->len=ntohs(p->len);
#endif 
	*/
	memcpy(b,p,6);
	b=&b[6];
	memcpy(b,p->data,len-6);
	b=buffer;
	
	do
	{
		do
		{
			if(len-tot<=0) break;
			FD_SET(fd,&fset);
			if(select(fd+1,NULL,&fset,NULL,NULL))	
			nwrite=write(fd,b,len-tot);
			FD_ZERO(&fset);
			myerror=errno;
			if(nwrite>0)
			tot=tot+(short)nwrite;
			if(tot<1024 && tot!=0)
			{
				b=buffer;
				b=&b[tot];
			}
			nwrite=0;			
		}while(nwrite<0 && myerror==EINTR);
	/*if(myerror>0)break;*/
	}while(tot<len);
	
#ifdef PROXY_DEBUG
	printf(" errore numero %d \n",nwrite);
	fflush(stdout);
#endif
	
	if(myerror && nwrite==-1)
		e_fatal("write session return an error",myerror);
	
	b=NULL;

#ifdef PROXY_DEBUG
	printf(GREEN"scritti %d byte \n"NORMAL,tot);
	fflush(stdout);
#endif
	
	free(buffer);
	buffer=NULL;	
	return tot;
}





int search_serial(l_list *ls,short int serial)
{ 
	int i=0;
	int len=0;
	l_list *tmp=NULL;
	struct ack_W *p=NULL;
	int res=-1;
	
	if(ls==NULL)
		e_fatal("NULL pointer",0);

#ifdef PROXY_DEBUG
	printf("search serial %d \n",ntohs(serial));
	fflush(stdout);
#endif
	
	len=q_length(ls);
	
#ifdef PROXY_DEBUG
	printf("len is %d \n",len);
	fflush(stdout);
#endif
	
	for(i=0;i<len;i++)
	{ 
		tmp=get_linked(i,ls);
		if(tmp==NULL)
			e_fatal("node is not in linked",0);
		p=(struct ack_W*)tmp->item;
		if(p==NULL)
			e_fatal("p can not be NULL \n",0);
		
		if(p->p->serial==serial)
		{
			res=i;
			tmp=NULL;
			break;
		}		
	}
	
#ifdef PROXY_DEBUG
	printf("fine search 	\n");
	fflush(stdout);
#endif
	return res;
}

l_list* read_ack(channel *canale, l_list *ack_list,int *ack_lock)
{
	byte read_buffer[6];
	byte* mybyte=NULL;
	struct timeval now;
	struct timeval diff;
	packet *p=NULL;
	int tot=0;
	int nread=0;
	int check=0;
	struct ack_W *ack=NULL;
	
#ifdef PROXY_DEBUG
	printf(RED"ON ACK\n"NORMAL);
	fflush(stdout);
#endif

	
	if(canale==NULL || ack_list==NULL)
		e_fatal("NULL pointer",0);
	

							
	memset(read_buffer,0x00,6);
	memset(&now,0x00,sizeof(struct timeval));
	/*i read one ack*/
	tot=0;
	mybyte=&read_buffer[0];
	do
	{
		do
		{
			nread=0;
			nread=read(canale->fd,mybyte,6-tot);
			if(nread<0)
				e_fatal("read failed",errno);
			if(nread)
					tot=tot+nread;
		}while((nread<0 && errno==EINTR) );
		
		if(nread==0)
			break;
		
		mybyte=&read_buffer[tot];

	}while(tot<6);
	
	if(nread==0)
	{
		canale->available=OFF;
		return ack_list;
	}
#ifdef PROXY_DEBUG
	printf(RED"READ ONE ACK \n"NORMAL);
	fflush(stdout);
	printf(RED"TOT IS %d \n"NORMAL,tot);
#endif
				
	if(tot==0)
	{
#ifdef PROXY_DEBUG
		printf(RED"no data\n"NORMAL);
		fflush(stdout);
#endif
		memset(read_buffer,0x00,6);
		return ack_list;
	}
							
	/*aproximation of recived data time*/
	gettimeofday(&now,NULL);
	normalizza(&now);
							
	p=new_packet(p);
								
	/*avoid to store read ack*/
								
								
	memcpy(&p->type,&read_buffer[0],2);
	memcpy(&p->serial,&read_buffer[2],2);
	memcpy(&p->len,&read_buffer[4],2);
							
#ifndef PROXY_BIG
	p->type=ntohs(p->type);
#endif
								
	if(p->type!=ACK)/*corrupted data*/
	{
		if(p->type==QUIT)
		{
		printf("sono su quit \n");
			canale->available=QUIT;
			p=delete_packet(p);
			return ack_list;
		}
#ifdef PROXY_DEBUG
		e_warning("corrupted data",0);
		printf(RED"p->type=%d \n"NORMAL,p->type);
		fflush(stdout);
#endif
		memset(read_buffer,0x00,6);
		p=delete_packet(p);
		return ack_list;
	}
	
		if(is_empty_linked(ack_list))
		return ack_list;
								
#ifndef PROXY_BIG
	p->type=htons(p->type);
#endif

	/*ack is stored*/
												
#ifdef PROXY_DEBUG
	printf("avoid to search ack\n");
	fflush(stdout);
	printf("p->type % d \n",p->type);
	fflush(stdout);	
	printf("p->serial is %d \n",ntohs(p->serial));
	fflush(stdout);
#endif
			
	/*search for recived ack*/
/*#ifndef PROXY_BIG*/
/*#else*/
	check=search_serial(ack_list,p->serial);
/*#endif*/

#ifdef PROXY_DEBUG
	printf(RED"check is %d \n"NORMAL,check);
	fflush(stdout);
#endif
									
	if(check==-1)/*packet not found*/
	{
		e_warning("no serial found",0);
		memset(read_buffer,0x00,6);
		return ack_list;
	}
	else	/* prima di cancellare prendere il tempo aggiornare le variabili 
							relative alla quantita' di byte disponibili inviabili e all'aumento e diminuzione di velociota'*/
	{
		memset(&diff,0x00,sizeof(struct timeval));
		
		/* get recived ack*/	
		ack=(struct ack_W*)get_linked(check,ack_list)->item;
										
		/*Estimate RTT*/
		diff=differenza(now,ack->send_time);
		normalizza(&diff);
#ifdef PROXY_DEBUG
		printf("differenza  sec=%d , usec=%d\n",diff.tv_sec,diff.tv_usec);
		fflush(stdout);
#endif
		
#ifdef PROXY_SEND_D
		if(*ack_lock)
		{		
			if(diff.tv_sec==0 && diff.tv_usec<800000)/*acceptable RTT or light delay*/
			{
				if(canale->score>600)
					canale->score+=64;
				else
					canale->score*=2;
				if(canale->score>1024)
					canale->score=1024;
				
			}
			else
			{
				canale->score=canale->score-(64*diff.tv_sec);
				if(canale->score<64)
					canale->score=64;
				
			}
		}
#else

										
		/*section where is modified score of channel*/
		if(*ack_lock)
		{		
			if(diff.tv_sec==0 && diff.tv_usec<800000)/*acceptable RTT or light delay*/
			{
			canale->score+=64;
			if(canale->score>1024)
				canale->score=1024;
				
			}
		else
		{
#ifdef PROXY_SEND_E
			canale->score-=128;
#else
			canale->score=canale->score-64;
#endif
			if(canale->score<64)
				canale->score=64;
				
		}
	}
		/*ifdef proxy send D*/

#endif  
		if(diff.tv_sec>0)
			*ack_lock=OFF;
		else
			*ack_lock=ON;

		
				
		/*end of section*/
		ack=NULL;								
		ack_list=delete_linked_node(ack_list,check);
		if(ack_list==NULL)/*if is the only ack remake linked_list*/
			ack_list=new_linked(ack_list);
		



		p=delete_packet(p);
		if(p!=NULL)
			e_fatal("i can not delete packet",0);

#ifdef PROXY_DEBUG	
		printf("I have finish ACK SESSION \n");
		fflush(stdout);
#endif
		
	}
	return ack_list;
}




l_list* read_data(channel *canale,l_list *data_queue)
{
	byte read_buffer[4096];
	int tot=0;
	int myerror=0;
	int nread=0;
	byte *mybyte=NULL;
	
	if(canale==NULL || data_queue==NULL)
		e_fatal(" NULL pointer",0);
	
	memset(read_buffer,0x00,4096);
	do
	{
		nread=read(canale->fd,read_buffer,4096);
		myerror=errno;
		if(nread<0)
			e_fatal("read failed",myerror);
	}while(nread<0 && myerror==EINTR);
	if(nread<0)
		e_fatal("read from client failed",myerror);
									
#ifdef PROXY_DEBUG
	printf(RED"Ho letto dei dati dal client \n"NORMAL);
	printf(GREEN" byte letti= %d \n"NORMAL,nread);
#endif
									
	if(nread==0)
	{
		canale->available=OFF;
		return data_queue;
	}
									
	mybyte=read_buffer;
	tot=0;
									
	do
	{
									
		if(nread>512)
		{
			push(mybyte,data_queue,512);
			tot+=512;
			mybyte=&read_buffer[tot];
			nread-=512;
		}
		else
		{
			push(mybyte,data_queue,nread);
			nread=0;
		}
	}while(nread>0);
							
#ifdef PROXY_DEBUG
	printf(GREEN"I have push data \n"NORMAL);
	fflush(stdout);
#endif
								
	nread=0;
	mybyte=NULL;
																
	return data_queue;
}

int sum_size(int fd,l_list *ls)
{
	l_list *tmp=NULL;
	unsigned int sum=0;
	struct ack_W *ack=NULL;
	
	if(ls==NULL)
		e_fatal("list must be != NULL",0);
	
	if(is_empty_linked(ls))
		return 0;
	
	tmp=ls;
	do
	{
		ack=(struct ack_W*)tmp->item;
		if(ack->where==fd)
		{
			if(sum<100000)
			sum+=ack->p->len;
			else
			{
				sum=10000;
				break;
			}
		}
			
		ack=NULL;
		tmp=tmp->next;		
	}while(tmp!=NULL);
	
	return sum;
}


queue* send_data(queue *q,l_list *ack_list,channel *canale,int *serial,int *res)
{
	byte buffer[1018];
	int i=0;
	int available=0;
	packet *p=NULL;
/*	int check=0;*/
	int myerror=0;
	byte * mybyte=NULL;
	struct ack_W *ack=NULL;
	int size=0;
	struct timeval now;
		
#ifdef PROXY_DEBUG
	printf("send data \n");
	fflush(stdout);
#endif
	
	if((q==NULL) || (ack_list==NULL) || (canale==NULL) || (res==NULL))
		e_fatal("NULL pointer send_data()",0);
	
	if(is_empty_q(q) || (canale->score<=6))
	{
		*res=0;
		return q;
	}
	
	memset(buffer,0x00,1018);
	available=canale->score;
	
	do
	{ 
		i=0;
		if(is_empty_q(q))
			break;
		
		if(available>=1024)
		{
			do
			{
	
				if(is_empty_q(q))
						break;
				
				size=get_isize(q);
				
				if(size+i>1018)
				{
					size=1018-i;
					/*copy available part*/
					memcpy(&buffer[i],q->item,size);
					q=delete_item(size,q);/*resize item in head*/
					i=1018;
					break;
				}
				else
				{
					q=cpy_pop(&buffer[i],q,&myerror,size);
					i+=size;
				}
			}while(i<1018);
		}
		else
		{
			while(i<available-6)
			{
				if(is_empty_q(q))break;
				size=get_isize(q);
				if(size+i>available-6)
				{
					size=available-6-i;
#ifdef PROXY_DEBUG
					printf("size=%d \n",size);
					fflush(stdout);
#endif
					/*print_node(q,0);*/
					memcpy(&buffer[i],q->item,size);
					q=delete_item(size,q);
					/*print_node(q,0);*/
					i=available-6;
					break;
				}
				else
				{
					q=cpy_pop(&buffer[i],q,&myerror,size);
					i+=size;
				}
			}
		}
		
	
				
		
		
		
		mybyte=NULL;
#ifdef PROXY_DEBUG
		printf("prima di send i is %d \n",i);
		fflush(stdout);
#endif
		if(i==0)
		{
			return q;
			break;
		}
		*serial=*serial+1;
		p=make_packet(DATA,*serial,i+6,buffer);
		if(p!=NULL)
			send_packet(canale->fd,p);
		else
		{
			*res=0;
			return q;
		}
#ifndef PROXY_BIG
		available-=ntohs(p->len);
#else
		available-=p->len;
#endif
		memset(&now,0x00,sizeof(struct timeval));
		gettimeofday(&now,NULL);
						
		ack=(struct ack_W*)malloc(sizeof(struct ack_W));
		if(ack==NULL)
			e_fatal("I can not allocate ack_W struct",0);
				
		memset(&ack->send_time,0x00,sizeof(struct timeval));						
		ack->p=NULL;
		ack->p=p;
		ack->send_time=now;						
		ack->where=canale->fd;
											
		point_push((void*)ack,(queue*)ack_list);
		if(is_empty_linked(ack_list))
			e_fatal("point push failed",0);
#ifdef PROXY_DEBUG
		printf("dopo point push \n");
		fflush(stdout);
#endif
		p=NULL;
		ack=NULL;
		
#ifdef PROXY_DEBUG
		printf("send queue ok \n");
		fflush(stdout);
#endif
		memset(buffer,0x00,1018);
		if(q==NULL)
			q=new_queue(q);
	}while(available>7 && is_empty_q(q)==0);
	
	
	*res=1;
	return q;			
	
}

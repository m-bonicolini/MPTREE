#include "Net_Util.h"

struct i_pack
{
	packet *p;
	int nread;
};

typedef struct i_pack i_packet;




i_packet* read_packet(channel * canale,i_packet* q)
{
	packet *p=NULL;
	byte buffer[1024];
	int nread=0;
	int tot=0;
	byte *mybyte=NULL;

#ifdef PROXY_DEBUG
	printf("start red packet \n");
	fflush(stdout);
#endif
	
	
	if(canale==NULL)
		e_fatal("wrong args channel",0);
	
	
	if(q==NULL)
	{
		memset(buffer,0x00,1024);
		p=new_packet(p);
		if(p==NULL)
			e_fatal("allocate packet",0);
		/*read header*/
		do
		{
			nread=0;
			do{
				nread=read(canale->fd,&buffer[tot],6-tot);
			}while(nread<0 && errno==EINTR);
			if(nread==0)
			{
				break;
			}
			if(nread<0)
				e_fatal("read() failed",errno);
			
			tot=tot+nread;
			if(6-tot<=0)break;
				
		}while(tot<6);

		if(nread==0)
		{
			canale->available=OFF;
			return q;
		}
		
		if(tot==6)
		{
			memcpy(&(p->type),&buffer[0],2);
#ifndef PROXY_BIG
			p->type=ntohs(p->type);
#endif
			
			memcpy(&(p->serial),&buffer[2],2);
#ifndef PROXY_BIG
			p->serial=ntohs(p->serial);
#endif
			
			memcpy(&(p->len),&buffer[4],2);
			
#ifndef PROXY_BIG
			p->len=ntohs(p->len);
#endif
		
			
			q=malloc(sizeof(i_packet));
			if(q==NULL)
				e_fatal("malloc fallita impossibile allocare i_packet",0);
#ifdef PROXY_DEBUG
			printf("p->len =%d \n",p->len);
			fflush(stdout);
#endif
		
			p->data=malloc(((p->len)-6));
			if(p->data==NULL)
				e_fatal("allocate data for packet",0);
		
			q->nread=6;
			q->p=p;
			p=NULL;
			return q;
		}
		else return q;
	}
	else
	{
		if(q->p->len==q->nread)
		{
#ifdef PROXY_DEBUG
			e_warning("packet is complete",0);
#endif
			return q;
		}
#ifdef PROXY_DEBUG
		else
			e_warning("complete packet",0);
#endif 
		
		nread=0;
		memset(buffer,0x00,1024);
		do
		{
			nread=read(canale->fd,buffer,(q->p->len)-(q->nread));
		}while(nread<0 && errno==EINTR);
		if(nread==0)
		{
			canale->available=OFF;
			return q;
		}
		
		if(nread<0)
			e_fatal("read failed",0);
		
	#ifdef PROXY_DEBUG
		printf("letti= %d \n",nread);
		fflush(stdout);
	#endif
		
		if(canale->fd==0)
			return q;
		
		mybyte=q->p->data;
			
		/*paranoid*/
		mybyte+=(q->nread)-6;
			
		memcpy(mybyte,buffer,nread);
		q->nread+=nread;

		
#ifdef PROXY_DEBUG
		printf("end read packet\n");
		fflush(stdout);
#endif
		
		return q;			
	}
			
}

int find_serial(l_list *ls,short int serial)
{
	l_list *tmp=NULL;
	packet *p=NULL;
	int i=0;
	int res=0;
	int len=0;
	
	if(ls==NULL)
		e_fatal("ls must be !=NULL",0);
	
	if(is_empty_linked(ls)==1)
		return -1;
	
#ifdef PROXY_DEBUG
	printf("search min \n");
	fflush(stdout);
#endif
	
	tmp=ls;
	
	len=q_length(ls);
	for(i=0;i<len;i++)
	{ 
		
		tmp=get_linked(i,ls);
		p=(packet*)tmp->item;
		if(p==NULL)
			e_fatal("p can not be NULL \n",0);
		
		if(p->serial==serial)
		{
			res=i;
			tmp=NULL;
			break;
		}
		p=NULL;
	}
	
	p=NULL;
	tmp=NULL;
	
	return res;
}


int write_ack(channel *canale,int serial)
{
	packet *ack=NULL;
	
	if(canale==NULL)
		e_fatal("canale must be !=NULL",0);
	
	ack=make_packet(ACK,serial,6,NULL);
	
	send_packet(canale->fd,ack);
	
	ack=delete_packet(ack);
	return 1;
	
}

int write_quit(int fd)
{
	packet *quit=NULL;
	

	
	quit=make_packet(QUIT,0,6,NULL);
	
	send_packet(fd,quit);
	
	quit=delete_packet(quit);
	return 1;
	
}

int check_down(int fd)
{
	int nread=0;
	char buffer=0;
	if(fd<=0)
		e_fatal("fd must be >0",0);
	
	nread=read(fd,&buffer,0);
	
	if(nread==0)
		return 0;
	else return 1;
}

void usage()
{
	printf("\n");
	msg_tutorial("./Proxy_reciver        start Proxy_sender with default value");
	msg_tutorial("./Proxy_reciver -help  print this help");
	msg_tutorial("./Proxy_reciver server_port and pair(ip port) for all channel");
	printf("\n");
	printf("\n");
}

void wrong_option(char *argv)
{	
	printf("\n");
	if(argv!=NULL)
	{
		printf(BLUE"["RED"BAD"BLUE"]"RED"  :unknow option %s\n"NORMAL,argv);
		fflush(stdout);
	}
	e_warning(" wrong option ",0);
	usage();
	e_fatal("can not start Proxy_reciver wrong option",0);
}

int main(int argc, char *argv[])
{
	
	l_list *packet_list=NULL;
	l_list *chan_list=NULL;
	l_list *tmp=NULL;
	channel *canale=NULL;
	fd_set fset;
	int check=0;
	int maxfd=0;
	SO_IN local;
	unsigned int lent=0;
	packet *p=NULL;
	short int now=1;
	int tot=0;
	int nwrite=0;
	i_packet *pak=NULL;
	int i=0;
	byte *mybyte=NULL;
	int myerror=0;
	int closed=0;
	int tmpfd=0;
	char *literals=NULL;
	
	chan_list=new_linked(chan_list);
	if(chan_list==NULL)
		e_fatal("allocate chan list",0);
	
	packet_list=new_linked(packet_list);
	if(packet_list==NULL)
		e_fatal("allocate packet list",0);
	
	switch(argc)
	{
		case 1:
		{
			e_warning("start with deafult value",0);
			
			/* Connect to Server */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->fd=proxy_connect("127.0.0.1",9001);
			if(canale->fd<0)
				e_fatal("connect() to server failed",0);
			canale->available=ON;
			point_push(canale,chan_list);
	
			canale=NULL;
			
			/* Listening on First Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->listenfd=proxy_listening("127.0.0.1",8001);
			canale->available=ON;
			point_push(canale,chan_list);
			canale=NULL;
			
			/* Listening on Second Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->listenfd=proxy_listening("127.0.0.1",8002);
			canale->available=ON;
			point_push(canale,chan_list);
			canale=NULL;
			
			/* Listening on Third Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->listenfd=proxy_listening("127.0.0.1",8003);
			canale->available=ON;
			point_push(canale,chan_list);
			canale=NULL;
		}break;
		case 2:
		{
			literals=argv[1];
			switch(strlen(argv[1]))
			{
				case 5:
				{
					if(literals[0]=='-'  && literals[1]=='h' && literals[2]=='e'
					   && literals[3]=='l' && literals[4]=='p')
					{
						literals=NULL;
						printf("\n");
						msg_help("select help option");
						usage();
						msg_ok("Proxy_sender is close");
						return 0;
					}
					else wrong_option(argv[1]);
				}break;
				default:
				{
					literals=NULL;
					wrong_option(argv[1]);
				}break;
			}
		}break;
		case 8:
		{
			/* Connect to Server */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->fd=proxy_connect("127.0.0.1",atoi(argv[1]));
			if(canale->fd<0)
				e_fatal("connect() to server failed",0);
			canale->available=ON;
			point_push(canale,chan_list);
			canale=NULL;
			tmp=NULL;
			
			/* Listening on First Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->listenfd=proxy_listening(argv[2],atoi(argv[3]));
			canale->available=ON;
			point_push(canale,chan_list);
			canale=NULL;
			
			/* Listening on Second Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->listenfd=proxy_listening(argv[4],atoi(argv[5]));
			canale->available=ON;
			point_push(canale,chan_list);
			canale=NULL;
			
			/* Listening on Third Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->listenfd=proxy_listening(argv[6],atoi(argv[7]));
			canale->available=ON;
			point_push(canale,chan_list);
			canale=NULL;
		}break;
		default:
		{
			e_fatal("Bad usage. Try ./Proxy_reciver -help",0);
		}break;
	}


	while(1)
	{
		if(closed>=3)
		{
			while(chan_list!=NULL)
			{
				canale=(channel*)chan_list->item;
				if(canale->fd)
				{
					write_quit(canale->fd);
					close(canale->fd);
				}
				if(canale->listenfd)
					close(canale->listenfd);
				chan_list=delete_linked_node(chan_list,0);
			}
			if(packet_list!=NULL)
				packet_list=delete_linked(packet_list);
			
			break;
		}
		
		FD_ZERO(&fset);
		
		
		tmp=chan_list;
		do
		{
			
			canale=(channel*)tmp->item;
			
			if(canale->listenfd && canale->fd==0)
			{	
				FD_SET(canale->listenfd,&fset);
				
				if(maxfd<canale->listenfd)
					maxfd=canale->listenfd;
			}
			
			if(canale->fd)
			{
				FD_SET(canale->fd,&fset);
			
				if(maxfd<canale->fd)
					maxfd=canale->fd;
			}
			
			tmp=tmp->next;
			
		}while(tmp!=NULL);
		
#ifdef PROXY_DEBUG	
		printf("select() \n");
		fflush(stdout);
#endif
		
		check=select(maxfd+1,&fset,NULL,NULL,NULL);

#ifdef PROXY_DEBUG
		printf(" get up\n");
		fflush(stdout);
#endif
		
		if(check)
		{
			tmp=chan_list->next;
			
			i=1;
			do
			{
				
				canale=(channel*)tmp->item;
				if(canale==NULL)
					e_fatal("allocate canale",0);
				
				/*i'm going to do accept*/
				if(canale->listenfd && FD_ISSET(canale->listenfd,&fset) && canale->fd==0)
				{
					do {
							memset (&local, 0x00, sizeof (local));
							lent = sizeof (local);
							canale->fd = accept (canale->listenfd, (struct sockaddr *) &local, &lent);
					}while ( (canale->fd<0) && (errno==EINTR));
			
					if(canale->fd<0) e_fatal("accept failed",errno);
					FD_CLR(canale->listenfd,&fset);	
					check--;
				}
				
				if(canale->fd && FD_ISSET(canale->fd,&fset) && canale->available)
				{	

					canale->pak=(i_packet*)read_packet(canale,canale->pak);
					if(canale->pak!=NULL)
					{
					
						pak=(i_packet*)canale->pak;
#ifdef PROXY_DEBUG
						printf("pak->len=%d pak->nread=%d \n",pak->p->len,pak->nread);
						fflush(stdout);
						printf("pak->p->serial %d \n",pak->p->serial);
						fflush(stdout);
#endif
						if(pak->nread==pak->p->len)
						{
						
#ifdef PROXY_DEBUG
							printf("faccio point push\n");
							fflush(stdout);
#endif

						
							point_push((pak->p),packet_list);
							write_ack(canale,pak->p->serial);
							pak->p=NULL;
							pak->nread=0;
							free(pak);
							canale->pak=NULL;
							pak=NULL;
						}
						else pak=NULL;
					}

					FD_CLR(canale->fd,&fset);
					check--;
				}
				
				
				if(canale->available==OFF)
				{
					close(canale->fd);
					canale->fd=0;
					e_warning("channel is down",0);
					closed++;
					if(canale->pak!=NULL)
					{
						free(canale->pak);
						canale->pak=NULL;
					}
					chan_list=delete_linked_node(chan_list,i);
					break;
				}
				i++;
				tmp=tmp->next;
				canale=NULL;
			}while(tmp!=NULL && check>0);
			
			canale=NULL;
			tmp=NULL;
			
			
		}
		else
			e_fatal("select() failed",errno);
		
		if(closed>=3)
			continue;
			
#ifdef PROXY_DEBUG
		printf("dopo select() \n");
		fflush(stdout);
#endif
		
		canale=NULL;
		tmp=NULL;
		

#ifdef PROXY_DEBUG
			printf("sort() \n");
			fflush(stdout);
#endif
			canale=(channel*)chan_list->item;
			if(FD_ISSET(canale->fd,&fset))
			{
				tmpfd=canale->fd;
				canale->fd=0;
				e_warning("server is down",0);
				closed=3;
				canale=NULL;
				if (chan_list->next!=NULL) 
				{
					while(tmp!=NULL)
					{
						tmp=chan_list->next;
						canale=(channel*)tmp->item;
					
						if(canale->fd)
						write_quit(canale->fd);
						
						canale=NULL;
						write_quit(canale->fd);
						
						tmp=tmp->next;
					}
				}
				close(tmpfd);
				tmpfd=0;
				continue;
			}
			
			if(closed==3)
				continue;
			
			while(is_empty_linked(packet_list)==0)
			{
#ifdef PROXY_DEBUG
				printf("sono nel while\n");
				fflush(stdout);
#endif
					
				canale=(channel*)chan_list->item;/*server channel*/
				
				i=0;
				i=find_serial(packet_list,now);
				if(i<0)
				{
#ifdef PROXY_DEBUG
					e_warning("linked is NULL",0);
#endif
					break;
				}
				
				tmp=get_linked(i,packet_list);
				
				p=(packet*)tmp->item;
				
#ifdef PROXY_DEBUG
				printf("minimal serial is %d \n",p->serial);
				fflush(stdout);
				printf("now is %d \n",now);
				fflush(stdout);
#endif
				if(p->serial==now)
				{
					
#ifdef PROXY_DEBUG
					e_warning("serial found",0);
#endif
					mybyte=p->data;
					tot=p->len-6;
					
					do
					{
						
						
						do {
						
							nwrite=0;
							myerror=0;
							nwrite=write(canale->fd,mybyte,tot);
							myerror=errno;
							
							tot-=nwrite;
							mybyte+=nwrite;
						}while(nwrite<0 || myerror==EINTR);
					}while(tot>0);
					
					tot=0;
					nwrite=0;
					mybyte=NULL;
					p=NULL;
					tmp=NULL;
					
					packet_list=delete_linked_node(packet_list,i);
					if(packet_list==NULL)
						packet_list=new_linked(packet_list);
		
					p=NULL;
					now++;
					
#ifdef PROXY_DEBUG
					printf("now is %d \n",now);
					fflush(stdout);
#endif
				}
				else
				{	
#ifdef PROXY_DEBUG
					e_warning("seriale not found",0);
#endif								
					p=NULL;

					break;
				}
				
				p=NULL;			
				canale=NULL;
			tmp=NULL;
			}				

		}/*end of while(1)*/
		/*}*/

	e_warning("exiting from Proxy_reciver",0);
	sleep(1);
	msg_ok("Proxy_reciver is quit");
	return 0;
		
}

#include <stdio.h>
#include <sys/stat.h>
#include "Net_Util.h"

int queue_size(queue *q,int sum)
{
	if(q==NULL)
		return sum;
	else
	{
		sum+=q->i_size;
		return queue_size(q->next,sum);
	}
	
}

int parse_port(char *string)
{
	if(string==NULL)
		e_fatal("string must be !=NULL",0);
		
	return atoi(string);
}


void print_strategy()
{
	int myok=0;
	
#ifdef PROXY_SEND_A
	if(!myok)
	{
		msg_ok("Proxy_sender is build with SENDA strategy");
		myok=1;
	}
	else
		e_fatal("you have select more than one option.You must recompile program",0);
#endif
	
#ifdef PROXY_SEND_B
	if(!myok)
	{
		msg_ok("Proxy_sender is build with SENDB strategy");
		myok=1;
	}
	else
		e_fatal("you have select more than one option.You must recompile program",0);
#endif
	
#ifdef PROXY_SEND_C
	if(!myok)
	{
		msg_ok("Proxy_sender is build with SENDC strategy");
		myok=1;
	}
	else
		e_fatal("you have select more than one option.You must recompile program",0);
#endif
	
#ifdef PROXY_SEND_D
	if(!myok)
	{
		msg_ok("Proxy_sender build with SENDD strategy");
		myok=1;
	}
	else
		e_fatal("you have select more than one option.You must recompile program",0);
#endif
	
#ifdef PROXY_SEND_E
	if(!myok)
	{
		msg_ok("Proxy_sender build with SENDE strategy");
		myok=1;
	}
	else
		e_fatal("you have select more than one option.You must recompile program",0);
#endif
	
#ifdef PROXY_SEND_F
	if(!myok)
	{
		msg_ok("Proxy_sender build with SENDF strategy");
		myok=1;
	}
	else
		e_fatal("you have select more than one option.You must recompile program",0);
#endif
	
	if(!myok)
		e_fatal("you have not select strategy for sending you must recompile program",0);
	
	
}

void usage()
{
	printf("\n");
	msg_tutorial("./Proxy_sender        start Proxy_sender with default value");
	msg_tutorial("./Proxy_sender -help  print this help");
	msg_tutorial("./Proxy_sender -man   print man page for Proxy_sender");
	msg_tutorial("./Proxy_sender client_port and pair(ip port) for all channel");
	printf("\n");
	msg_help("use Proxy_sender -man for more info");
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
	e_warning("You have pass wrong option ",0);
	usage();
	e_fatal("can not start Proxy_sender wrong option",0);
}

static void p_man(FILE *f)
{
	int nread=0;
	char *line=NULL;
	int i=0;
	struct  stat s;
	
	if(f==NULL)
		e_fatal("you must pass path of man pages",0);
	
	if(fstat(fileno(f),&s)==-1)
		e_fatal("fstat failed",errno);
	
	if(s.st_size)
	{
		if(s.st_size%2==0)
		{
			line=malloc(s.st_size);
		}
		else
		{
			line=malloc(s.st_size+1);
		}
	}
	else
		e_fatal("can not estimate size of man pages",0);
	
	do
	{
		nread=read(fileno(f),line,s.st_size);
	}while(nread<0 && errno==EINTR);
	if(nread<0)
		e_fatal("read failed",0);
	
	for (i=0;line[i]!='\0';i++)
	{
		printf("%c",line[i]);
		fflush(stdout);
	}
}

void print_man()
{
	FILE *f=NULL;
	
	msg_help("you have run ./Proxy_sender -man");
	
	f=fopen("./docs/man.txt","r");
	if(f==NULL)
		e_fatal("fopen() failed",errno);
	p_man(f);
	fclose(f);
}

int main(int argc ,char *argv[])
{
	
	l_list *tmp_list=NULL;
	channel *canale=NULL;
	l_list *chan_list=NULL;
	l_list *ack_list=NULL; 
	l_list *losed=NULL;
	static fd_set fset;
	static fd_set wset;	
	static int proposition=0;
	struct ack_W *ack=NULL;
	queue *data_queue=NULL;
	static int serial=0;
	static int check=0; 
	static int maxfd=0;   
	TIME *s_time=NULL;
	static SO_A local;
	static int myerror=0;
	static int len=(int)sizeof(local);
	static TIME now;
	static int first=1;
	static int myread=ON;
	static int congestioned=0;
	static int i=0;
	static int mywrite=ON;
	static int closed=0;
	static int lose=0;
	static TIME scade;
	static int problem=OFF;
	char *literals=NULL;
	
	
	print_strategy();
	
	data_queue=new_queue(data_queue);
	if(data_queue==NULL)
		e_fatal("data queue must be !=NULL",0);
	
	chan_list=new_linked(chan_list);
	if(chan_list==NULL)
		e_fatal("no space for allocate chan_list",0);
	
	ack_list=new_linked(ack_list);
	if(ack_list==NULL)
		e_fatal("no space for allocate linked",0);

	scade.tv_sec=4;
	scade.tv_usec=0;
	
	switch(argc)
	{
		case 1:
		{
			e_warning("Proxy_sender start with deafult value",0);

			/*listening to client*/
			
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
			canale->listenfd=proxy_listening("127.0.0.1",6001);
			canale->available=ON;
			proxy_sock_opt(canale->listenfd ,SO_KEEPALIVE,1,&myerror);
			myerror=0;
			canale->port=6001;
			point_push(canale,chan_list);
			canale=NULL;
	
			/*connect to 3 channel*/
	
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal(" space for allocate canale",0);
	
			canale->fd=proxy_connect("127.0.0.1",7001);
			canale->available=ON;
			canale->score=1024;
			canale->success=ON;
			canale->ack_lock=ON;
			myerror=0;
			canale->port=7001;
			point_push(canale,chan_list);
			canale=NULL;
	
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->fd=proxy_connect("127.0.0.1",7002);
			canale->available=ON;
			canale->score=1024;
			canale->success=ON;
			canale->ack_lock=ON;
			myerror=0;
			canale->port=7002;
			point_push(canale,chan_list);
	
			canale=NULL;
	
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->fd=proxy_connect("127.0.0.1",7003);
			canale->available=ON;
			canale->score=1024;
			canale->success=ON;
			canale->ack_lock=ON;
			myerror=0;
			canale->port=7003;
			point_push(canale,chan_list);
			canale=NULL;
	
		}break;
		case 2:
		{
			literals=argv[1];
			switch(strlen(argv[1]))
			{
				case 4:
				{
					if(literals[0]=='-'  && literals[1]=='m' && literals[2]=='a'
					   && literals[3]=='n')
					{
						print_man();
						printf("\n");
						fflush(stdout);
						return 0;
					}break;
				}break;
				case 5:
				{
					if(literals[0]=='-'  && literals[1]=='h' && literals[2]=='e'
					   && literals[3]=='l' && literals[4]=='p')
					{
						literals=NULL;
						printf("\n");
						msg_help("help option");
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
			
			/* Client Listening Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
			
			canale->listenfd=proxy_listening("127.0.0.1",atoi(argv[1]));
			canale->available=ON;
			proxy_sock_opt(canale->listenfd,SO_KEEPALIVE,1,&myerror);
			myerror=0;
			canale->port=atoi(argv[1]);
			point_push(canale,chan_list);
			canale=NULL;
			
			/* First Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->fd=proxy_connect(argv[2],atoi(argv[3]));
			canale->available=ON;
			canale->score=1024;
			canale->success=ON;
			canale->ack_lock=ON;
			myerror=0;
			canale->port=atoi(argv[3]);
			point_push(canale,chan_list);
			canale=NULL;
			
			/* Second Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->fd=proxy_connect(argv[4],atoi(argv[5]));
			canale->available=ON;
			canale->score=1024;
			canale->success=ON;
			canale->ack_lock=ON;
			myerror=0;
			canale->port=atoi(argv[5]);
			point_push(canale,chan_list);
			canale=NULL;
			
			/* Third Remote Channel */
			canale=new_channel(canale);
			if(canale==NULL)
				e_fatal("space for allocate canale",0);
	
			canale->fd=proxy_connect(argv[6],atoi(argv[7]));
			canale->available=ON;
			canale->score=1024;
			canale->success=ON;
			canale->ack_lock=ON;
			myerror=0;
			canale->port=atoi(argv[7]);
			point_push(canale,chan_list);
			canale=NULL;
		}break;
		default:
		{
			e_fatal("Bad usage. Try ./Proxy_sender -help",0);
		}break;
	}
	
	/*Proxy_sender loop*/
	while(1)
	{
		/*all channel are closed or i recive quit message*/
		if(closed>=3 || problem)
		{
			
			/*delete all chan_list*/
			while(chan_list!=NULL)
			{
				canale=(channel*)chan_list->item;
				if(canale->fd)
				{
					close(canale->fd);
					canale->fd=0;
				}
				canale=NULL;
				chan_list=delete_linked_node(chan_list,0);
			}
			
			if(ack_list!=NULL)
				ack_list=delete_linked(ack_list);
			if(losed!=NULL)
				losed=delete_linked(losed);
			break;
		}
		
		if(is_empty_linked(chan_list) || chan_list==NULL)
			break;
		
		
		FD_ZERO(&fset);
		FD_ZERO(&wset);

		
		/*get channel than comunicate with client*/
		canale=(channel*)((l_list *)get_linked(0,chan_list))->item;
		if(canale==NULL)
			e_fatal("find channel",0);
		
		if(first==ON)
		{
			if(canale->listenfd)
			{
				FD_SET(canale->listenfd,&fset);
				if(maxfd<canale->listenfd)
					maxfd=canale->listenfd;
			}
		}
		else
		{
			if(myread)
			{
				if(canale->fd)
				{
					FD_SET(canale->fd,&fset);
					if(canale->fd>maxfd)
						maxfd=canale->fd;
				}
			}
	
			tmp_list=chan_list->next;
			while(tmp_list!=NULL)
			{
				canale=(channel*) tmp_list->item;
				if(canale->fd)
				{
					FD_SET(canale->fd,&fset);
						
					if(first==OFF && mywrite)
						FD_SET(canale->fd,&wset);
						
					if(canale->fd>maxfd)
						maxfd=canale->fd;
				}
				tmp_list=tmp_list->next;
				canale=NULL;
			}
			
		}
		scade.tv_sec=4;
		scade.tv_usec=0;

		if(is_empty_q(data_queue)==0 && mywrite )
			check=select(maxfd+1,&fset,&wset,NULL,NULL);
		else
			check=select(maxfd+1,&fset,NULL,NULL,&scade);


		myerror=errno;
		maxfd=0;
		
#ifdef PROXY_DEBUG
		printf("get up \n");
		fflush(stdout);
#endif
		switch(check)
		{
			case -1: e_fatal("select() failed",myerror);
			default:
			{
				myerror=0;
				
				canale=(channel*)((get_linked(0,chan_list))->item);
				
				if(first && canale->listenfd)
				{
					if(FD_ISSET(canale->listenfd,&fset))
					{
						do
						{
							memset(&local,0x00,sizeof(local));
							canale->fd=accept(canale->listenfd,(SO_A*)&local,(socklen_t*)&len);
						}while((canale->fd<0) && errno==EINTR);
							
						if(canale->fd<0)
							e_fatal("accept failed",errno);
						
					#ifdef PROXY_DEBUG
						printf("accept ok \n");
						fflush(stdout);
					#endif
						
						FD_CLR(canale->listenfd,&fset);
						check--;
						
					#ifdef PROXY_DEBUG
						printf("check is %d \n",check);
						fflush(stdout);
                    #endif
						
						canale=NULL;
						first=OFF;
						continue;
					}
					proposition=0;
				}
				
				if(check && canale->fd)
				{
					if(FD_ISSET(canale->fd,&fset) && myread)
					{
						data_queue=read_data(canale,data_queue);
						if(data_queue==NULL)
							e_fatal("read_data() failed",0);
						
						if(canale->available==OFF)
						{
							e_warning("client close his connection",errno);
							e_warning("program close connections",0);
							
							canale=(channel*)chan_list->item;
							if(canale==NULL)
								e_fatal("canale must be !=NULL",0);
							
							close(canale->listenfd);
							close(canale->fd);
							free(canale);
							canale=NULL;
							chan_list->item=NULL;
							chan_list=delete_linked(chan_list);
							ack_list=delete_linked(ack_list);
							data_queue=delete_q(data_queue);
							FD_ZERO(&fset);
							FD_ZERO(&wset);
							continue;
						}
						FD_CLR(canale->fd,&fset);
						check--;
					}
					canale=NULL;
				}

				if(chan_list->next!=NULL)
				{
					tmp_list=chan_list->next;
					i=1;
					while(tmp_list!=NULL && check>0)
					{
						canale=(channel*)tmp_list->item;
						if(canale==NULL)
							e_fatal("channel not found",0);

						if(canale->fd)
						{
            #ifdef PROXY_DEBUG
							printf(RED"ack session \n"NORMAL);
							fflush(stdout);
            #endif
							if(FD_ISSET(canale->fd,&fset))
							{
								ack_list=read_ack(canale,ack_list,&(canale->ack_lock));
								if(ack_list==NULL)
									e_fatal("read_ack() failed",0);
								
								if(canale->available==QUIT)
								{
									closed=3;
									problem=ON;
									e_warning("quit message from Proxy_reciver",0);
									canale=NULL;
									break;
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
									problem=ON;
									tmp_list=chan_list->next;
									i=1;
									continue;
								}
								
								FD_CLR(canale->fd,&fset);
								--check;
							}/*IF FD_ISSET()*/

						}/*if canale->fd*/
					    if(closed==3) break;

						tmp_list=tmp_list->next;
						i++;
						canale=NULL;
					}/*while(tmp_list!=NULL)*/
					if(closed==3) continue;
						
						tmp_list=chan_list->next;
						while(tmp_list!=NULL)
						{
							canale=(channel*)tmp_list->item;
							if(canale==NULL)
								e_fatal("channel not found",0);

							if(canale->fd)
							{
								if(FD_ISSET(canale->fd,&fset))
								{
									close(canale->fd);
									canale->fd=0;
									e_warning("channel is down",0);
									closed++;
									canale=NULL;
								}
							}
							tmp_list=tmp_list->next;
						}
				
				    if(is_empty_q(data_queue)==0 && mywrite && problem==OFF)
				    {
				    
 			#ifdef PROXY_DEBUG
					printf(RED"WRITE SESSION \n"NORMAL);
					fflush(stdout);
			#endif
						i=1;
						tmp_list=chan_list->next;
						while(tmp_list!=NULL)
						{
							canale=(channel*)tmp_list->item;
							if(canale==NULL)
								e_fatal("channel not found",0);
					
							if(canale->score<7)
							{
								i++;
								tmp_list=tmp_list->next;
								continue;
							}
							
							myerror=0;
							if(canale->fd)
							{
								if(FD_ISSET(canale->fd,&wset) && canale->success)
								{
									data_queue=send_data(data_queue,ack_list,canale,&serial,&myerror);	
									if(!myerror)
									{
                           #ifdef PROXY_DEBUG
										e_warning("send_data() failed",0);
			               #endif					
										canale->success=OFF;
									}
						   #ifdef PROXY_DEBUG
									printf(RED"end of send_data() \n"NORMAL);
									fflush(stdout);
						   #endif	
								}/*if fd is set*/
							}/*if canale->fd*/
                            i++;
							tmp_list=tmp_list->next;
						}/*while(tmp->list!=NULL)*/
					}/*if(is_empty_q(data_queue)==0*/
			}/*if(chan_list->next!=NULL)*/
            else break;

		}/*default*/break;
	}/*switch(check)*/
	maxfd=0;
	

	/*prima di qui solo read_ack ha modifcato la grandezza del pacchetto in base all'ack ricevuto
	  mi accingo a fare valutazioni + precise*/
	if(closed==3) continue;

	if(is_empty_linked(ack_list)==0)/*se ci sono pacchetti che aspettano ack*/
	{
		s_time=malloc(sizeof(TIME));
		memset(s_time,0x00,sizeof(TIME));
		ack=(struct ack_W*)ack_list->item;
		memset(&now,0x00,sizeof(TIME));
		gettimeofday(&now,NULL);
		*s_time=differenza(now,ack->send_time);
		normalizza(s_time);
		/*se da quando e' stato ivniato il primo pacchetto nella lista di ack e' passato meno tempo di*/
		if(s_time->tv_sec==0 && s_time->tv_usec<=900000 )
		{		
			myread=ON;/*posso mettere l'fd del client sulla select poiche' voglio aumetare 
			la velocita' e' necessario leggere dati*/
	

			/*imposto il valore massimo di congestione a quello (supposto) corrente
			basandomi sul traffico complessivo (byte che aspettano ack(quindi inviati) 
			ee li sommo agli ack virtualmente in rete che stanno per arrivare*/
#if defined(PROXY_SEND_C) || defined(PROXY_SEND_B) || defined(PROXY_SEND_D)
			congestioned=(queue_size(ack_list,0)+(q_length(ack_list)*6));
#endif
			mywrite=ON;
			tmp_list=chan_list->next;
			i=0;
			do {
				canale=(channel*) tmp_list->item;
				if(canale==NULL)
					e_fatal("no channel",0);
				
#if   defined(PROXY_SEND_A) || defined(PROXY_SEND_E) || defined(PROXY_SEND_F)
				if(canale->fd==ack->where)
				{

					if(canale->ack_lock)
					{
						canale->success=ON;
						congestioned=(queue_size(ack_list,0)+(q_length(ack_list)*6));
						break;
					}
					
				}
#else
				
   #if defined(PROXY_SEND_C) || defined(PROXY_SEND_B) || defined(PROXY_SEND_D)
				if(canale->fd==ack->where)
				{
					canale->success=ON;
					canale->ack_lock=ON;
					break;
				}
   #endif
				
#endif
				i++;
				tmp_list=tmp_list->next;
				canale=NULL;
			} while (tmp_list!=NULL);
			tmp_list=NULL;
			canale=NULL;
		}
		else
		{
			tmp_list=chan_list->next;
			i=1;
			do{
				canale=(channel*) tmp_list->item;
				if(canale==NULL)
					e_fatal("no channel",0);
				
				if(canale->fd==ack->where)
				{
#if defined(PROXY_SEND_C) || defined(PROXY_SEND_D) || defined(PROXY_SEND_B) || defined(PROXY_SEND_E)|| defined (PROXY_SEND_F)
					canale->ack_lock=ON;
#endif
					
#ifndef PROXY_SEND_A
					canale->success=OFF;
#endif			
					if(s_time->tv_sec>9)
					{
						e_warning("Proxy_sender: connection timeout",0);
						close(canale->fd);
						canale->fd=0;
						closed++;
						lose=ON;
						
						if(losed==NULL)
							losed=new_linked(losed);
						
						while(s_time->tv_sec>9)
						{
							point_push((void*)ack_list->item,(queue*)losed);
							ack_list->item=NULL;
							
							ack_list=delete_linked_node(ack_list,0);
							if(is_empty_linked(ack_list)==0)
							{
								memset(s_time,0x00,sizeof(TIME));
								memset(&now,0x00,sizeof(TIME));
								gettimeofday(&now,NULL);
								*s_time=differenza(now,ack->send_time);
								continue;
							}
							else 
							{
								if(ack_list==NULL)
									ack_list=new_linked(ack_list);
								break;
							}
						}
						mywrite=ON;
						canale=NULL;
						chan_list=delete_linked_node(chan_list,i);
						break;
					}/*if s_time->tv_sec>9 */

				}
			i++;
			tmp_list=tmp_list->next;
			canale=NULL;
			}while(tmp_list!=NULL);
			tmp_list=NULL;
			canale=NULL;
            ack=NULL;
		}
		free(s_time);
		s_time=NULL;
		if(closed>=3)continue;
     }
	else
	{
		myread=ON;
		canale=NULL;
		tmp_list=NULL;
		tmp_list=chan_list->next;
		do{
			canale=(channel*) tmp_list->item;
			if(canale==NULL)
				e_fatal("no channel",0);
	
			canale->success=ON;
#ifndef PROXY_SEND_A
			canale->ack_lock=ON;
#endif
			tmp_list=tmp_list->next;
			canale=NULL;
		}while(tmp_list!=NULL);
		tmp_list=NULL;
		canale=NULL;
	}



/*se i byte rpesenti nella lista di pacchetti che attendono ACK e 
	i byte nella coda di lettura dal client sono maggiori di congestioned */
	
	if(queue_size(ack_list,0)+queue_size(data_queue,0)>congestioned)
	{
		myread=OFF;/*non leggo dati dal client la prossima select*/
		tmp_list=chan_list->next;
		mywrite=OFF;/*valore di partenza per il ciclo*/
			do {
				canale=(channel*) tmp_list->item;
				if(canale==NULL)
					e_fatal("no channel",0);
				mywrite=(mywrite|| canale->success);/*setto il permesso di scrittura globale*/
				canale->ack_lock=ON;
				tmp_list=tmp_list->next;
				canale=NULL;
			} while (tmp_list!=NULL);
			tmp_list=NULL;
			canale=NULL;
	}
	else
	{
		tmp_list=NULL;
		canale=NULL;
		myread=ON;
		if(chan_list!=NULL)
		tmp_list=chan_list->next;
		mywrite=ON;
		while(tmp_list!=NULL)
		{
			canale=(channel*)tmp_list->item;
			canale->success=ON;
			tmp_list=tmp_list->next;
		}
		tmp_list=NULL;
		canale=NULL;
		

	}

#ifdef PROXY_SEND_B
    tmp_list=chan_list->next;
    canale=NULL;
	while(tmp_list!=NULL)
	{
		canale=(channel*)tmp_list->item;
		if(canale->ack_lock==OFF)
			congestioned=congestioned/3;
		
		tmp_list=tmp_list->next;
	}
	tmp_list=NULL;
	canale=NULL;
#endif


if(losed==NULL)
	lose=OFF;


if(lose && closed<3)
{
	
#ifdef PROXY_DEBUG
	printf("packet lost \n");
	fflush(stdout);
#endif
	
	tmp_list=chan_list->next;
	canale=(channel*)tmp_list->item;
	
#ifdef PROXY_DEBUG
	printf("prendo losed \n");
	fflush(stdout);
#endif
	ack=(struct ack_W*)losed->item;
	
	if(ack->p==NULL)
		e_fatal("losed packet is NULL",0);
	
#ifdef PROXY_DEBUG		
	printf("prima di send() \n");
	fflush(stdout);
	printf("canale->fd= %d \n",canale->fd);
	fflush(stdout);
#endif
	send_packet(canale->fd,ack->p);
	
#ifdef PROXY_DEBUG
	printf("after send_packet()\n");
	fflush(stdout);
#endif
	
	memset(&ack->send_time,0x00,sizeof(TIME));						
	gettimeofday(&(ack->send_time),NULL);
	ack->where=canale->fd;
	
	point_push((void*)ack,(queue*)ack_list);
	if(is_empty_linked(ack_list))
		e_fatal("point push failed",0);
	
	losed->item=NULL;
	losed=delete_linked_node(losed,0);
	canale=NULL;
	while(tmp_list!=NULL)
	{
		canale=(channel*)tmp_list->item;
		canale->success=ON;
		canale=NULL;
		tmp_list=tmp_list->next;
	}
}

memset(&proposition,0x00,sizeof(int));
proposition=is_empty_linked(ack_list) && is_empty_q(data_queue);
if(proposition)
	myread=ON;

}/*while(1)*/

e_warning("Proxy_sender is closing",0);
sleep(1);

msg_ok("Proxy_sender is close");

	return 0;
}

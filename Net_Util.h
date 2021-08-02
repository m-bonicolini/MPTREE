#ifdef PROXY_BSD
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <strings.h>
  #include <unistd.h>
#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#endif

#include <sys/time.h>
#include <netinet/tcp.h>

#include <fcntl.h>
#include "l_list.h"
#include "Channel.h"
#include "Packet.h"

#define SEC_IN_MCSEC 1000000L

#define ON 		1
#define OFF 	0

typedef struct sockaddr_in SO_IN;
typedef struct sockaddr	SO_A;
typedef struct timeval TIME; 



struct ack_W 
{
	packet *p;
	struct timeval send_time;
	int where;
};

extern int proxy_sock_opt(int socket ,int socket_op,int value,int* myerror);/*set*/
extern int proxy_tcp_opt(int socket,int socket_op,int value,int *myerror);/*set*/
extern int proxy_listening(char *remote_IP,int port);/*do listen() with our default parameters*/
extern int advanced_listening(char* remote_IP,int port,int s_buff,int r_buff);
extern int proxy_connect(char *remote_IP,int port);/*do connect() with our default parameters*/
extern int advanced_connect(char* remote_IP,int port,int s_buff,int r_buff);
extern int set_tcpnodelay(int fd);
extern int disable_tcpnodelay(int fd);
extern int normalizza( struct timeval *t );
extern int somma(struct timeval tmr,struct timeval ist,struct timeval *delay);
extern struct timeval differenza(struct timeval dopo,struct timeval prima);
extern int minore(struct timeval *a, struct timeval *b);
extern void stampa_timeval(char *str, struct timeval tv);
extern int send_packet(int fd,packet *p);/*write() packet in fd*/
extern int search_serial(l_list *ls,short int serial);/*search for packet "serial" in list "ls"*/

extern l_list* read_ack(channel *canale, l_list *ack_list,int *ack_lock);/*read() ack from channel "canale"*/

extern l_list* read_data(channel *canale,l_list *data_queue);/*read() data from "canale" 
																					and store in data_queue*/

extern int sum_size(int fd,l_list *ls);/*sum size of all packet with the specified fd in the list*/

extern queue* send_data(queue *q,l_list *ack_list,channel *canale,int *serial,int *res);/*create a packet, 
																				send it in "canale" and store it in "ack_list"*/

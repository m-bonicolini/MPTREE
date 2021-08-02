GCCFLAGS= -Wall  -pedantic -ggdb


all: Proxy_configure Proxy_sender Proxy_reciver

Proxy_configure: Proxy_Configure.o
		gcc -o Proxy_configure ${GCCFLAGS} Proxy_Configure.o
		./Proxy_configure LINUX   SENDC


Proxy_configure.o:Proxy_Configure.c
		gcc -c ${GCCFLAGS} Proxy_configure.c

Proxy_sender:	Proxy_sender.o Message.o queue.o l_list.o Channel.o Packet.o Net_Util.o		
		gcc -o Proxy_sender ${GCCFLAGS}   Proxy_sender.o   Message.o queue.o l_list.o Channel.o Packet.o Net_Util.o

Proxy_sender.o: Proxy_sender.c Net_Util.h
		gcc -c  ${GCCFLAGS} Proxy_sender.c

Proxy_reciver:   Proxy_reciver.o Message.o queue.o l_list.o Channel.o Packet.o Net_Util.o	
		gcc -o Proxy_reciver ${GCCFLAGS} Proxy_reciver.o Message.o queue.o l_list.o Channel.o Packet.o Net_Util.o

Proxy_reciver.o: Proxy_reciver.c Net_Util.h 
		gcc -c  ${GCCFLAGS} Proxy_reciver.c
			 
Net_Util.o:      Net_Util.c  l_list.h Packet.h Channel.h	
		
		gcc -c ${GCCFLAGS} Net_Util.c 
		
l_list.o:       l_list.c queue.h queue.o
		gcc -c  ${GCCFLAGS} l_list.c
		
queue.o:        queue.c Message.h Message.o
		gcc -c ${GCCFLAGS} queue.c

Channel.o:      Channel.c  Message.h Message.o
		gcc -c ${GCCFLAGS} Channel.c

Packet.o:       Packet.c  Message.h Message.o
		gcc -c  ${GCCFLAGS} Packet.c
				
Message.o:      Message.c Proxy_config.h
		gcc -c ${GCCFLAGS} Message.c

clean:
	rm -f *.o
	rm -f Proxy_sender
	rm -f Proxy_reciver
	rm -f Proxy_configure
	clear 


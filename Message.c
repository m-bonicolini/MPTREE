#ifndef _MESSAGE_C
#define _MESSAGE_C
#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Message.h"


#ifndef MESSAGE_H
#error No Message.h header file
#endif
/*Function for warning message*/
void e_warning(const char *msg,int myerror)
{
	if(msg==NULL) /*No developer msg*/
	{
		if(myerror>0)  /*error value refer to errno*/
		{
			printf(BLUE"["YELLOW"WARNING"BLUE"]"
				  	  YELLOW" : %s  \n"NORMAL,	strerror(myerror));
			fflush(stdout);
			return;
		}
		else /*bad errno value print ifo msg*/
		{
			printf(BLUE"["YELLOW"WARNING"
				 	  BLUE"]");
			printf("\t"YELLOW
						 ": e_warning(char* msg,int error)"
						"bad usage of int error \n"NORMAL);
			fflush(stdout);
			return;
		}
	}
	else		/*There is a developer msg*/
	{
		if(myerror>0)	/*errno value >0*/
		{
			printf(BLUE"["YELLOW"WARNING"
				 	 BLUE"]"YELLOW" : %s \n"NORMAL,msg);
			fflush(stdout);
			printf(BLUE"["YELLOW"WARNING"BLUE"]"
				 	 YELLOW" : %s \n"NORMAL,strerror(myerror));
			fflush(stdout);
			return;
		}
		else
		{
			if(myerror<0) /*errno<0 value print
									ifo msg*/
			{
				printf(BLUE"["YELLOW"WARNING"
				 	      BLUE"]");
				printf("\t"YELLOW
						 " : e_warning(char* msg,int error)"
						"bad usage of int error \n"NORMAL);
				fflush(stdout);
			}

			printf(BLUE"["YELLOW"WARNING"
				 	 BLUE"]"YELLOW"\t: %s \n"NORMAL,msg);
			fflush(stdout);
			return;
		}
	}
}


/*Function for fatal error message*/
void e_fatal(const char *msg ,int myerror)
{
	if(msg==NULL) /*No developer msg*/
	{
		if(myerror>0) /*errno value >0*/
		{
			printf(BLUE"["RED"ERROR"BLUE"]"
					 RED " : %s\n"NORMAL,strerror(myerror));
				printf(NORMAL);
			fflush(stdout);
			exit(1);		/*exit from main program*/
			return;   /* paranoid*/
		}
		else /*errno value <0 print info msg*/
		{
			printf(BLUE"["RED"ERROR"BLUE"]"RED
					  " : e_fatal() bad usage of int error\n"NORMAL);
			printf(NORMAL);
			fflush(stdout);
			exit(1);		/*exit from main program*/
			return;	/*Paranoid*/
		}
	}
	else /*There is developer msg*/
	{
		if(myerror>0) /*errno value >0*/
		{
			printf(BLUE"["RED"ERROR"BLUE"]"RED
					  " : %s\n"NORMAL,msg);
			printf(BLUE"["RED"ERROR"BLUE"]"RED
					 " : %s\n"NORMAL,strerror(myerror));
			fflush(stdout);
			exit(1);		/*exit from main program*/
			return;	/*Paranoid*/
		}
		else	/*errno value <=0*/
		{
			printf(BLUE"["RED"ERROR"BLUE"]"RED
					 " : %s\n"NORMAL,msg);
			if(myerror<0)	/*errno value <0
									 print info message*/
			{
				printf(BLUE"["RED"ERROR"BLUE"]"RED
					  " : e_fatal() bad usage of int error\n"NORMAL);
			}
			fflush(stdout);
			exit(1); /*exit from main program*/
			return ; /*paranoid*/
		}
	}
}

/*Function for print ok message*/
void msg_ok(const char *msg)
{
	if(msg==NULL)
	{
		e_fatal("Bad usage of msg_ok()",0);
		return;/*paranoid*/
	}
	else
	{
		printf(BLUE"["GREEN"OK"BLUE"]"GREEN
				  " : %s\n"NORMAL,msg);
		fflush(stdout);
		return;
	}
}

/*Function for help message*/
void msg_help(const char *msg)
{
	if(msg==NULL)
	{
		e_fatal("Bad Usage of msg_help()",0);
		return;
	}
	else
	{
		printf(BLUE"[""HELP""]"
				  " : %s \n"NORMAL,msg);
		printf(NORMAL);
		fflush(stdout);
		return;
	}
}

/*Function for tutorial message*/
void msg_tutorial(const char *msg)
{
	if(msg==NULL)
	{
		e_fatal("Bad usage of msg_tutorial()",0);
		return;
	}
	else
	{
		printf(BLUE"["MAGENTA"USAGE"BLUE
					"]"MAGENTA" : %s \n"NORMAL,msg);
		fflush(stdout);
		return;
	}
}


#ifdef __STDC__

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

/*#include "Message.h"*/

#include "Message.c"

static int strategy=0;
#define LP32   32
#define ILP32  33
#define LP64   64
#define ILP64  65
#define NOT_SUPPORTED -1

#define IS_32BIT(cpu) (cpu==LP32 || cpu==ILP32)
#define IS_64BIT(cpu) (cpu==LP64 || cpu==ILP64)


/* Assumed char==8 bits*/
static int os_cpu_model()
{
	const unsigned short int my_int=sizeof(unsigned int);
	const unsigned short int my_long=sizeof(unsigned long);
	const unsigned short int my_pointer=sizeof(void*);

	if((my_int==2)&&(my_long==4)&&(my_pointer==4))		return LP32;  	/*LP32 model*/
	
	if((my_int==4)&&(my_pointer==4)&&(my_long==4))		return ILP32; 	/*ILP32 model*/
	
	if((my_int==4)&&(my_pointer==8)&&(my_long==8))		return LP64;	/*LP64 model*/
	
	if((my_int==8)&&(my_pointer==8)&&(my_long==8))		return ILP64;	/*ILP64 model*/
	
	return NOT_SUPPORTED; /*error*/
}

/*This function return 1
	if cpu is BIG ENDIAN */
static int is_big()
{
	const int p_model=os_cpu_model();
	int i=0;
	unsigned long int number=0;
	unsigned char *p=NULL;

	if(p_model==NOT_SUPPORTED) return NOT_SUPPORTED;

	number=1;
	p=(unsigned char*)&number;
	
	if(IS_32BIT(p_model))
	{
		if(p[3]==1 && p[2]==0 && p[1]==0 && p[0]==0) 
		return 1;  /*|0|0|0|1| big endian*/
	}
	
	if(IS_64BIT(p_model))
	{
		if(p[7]==1)			
		{
			for(i=6;i>=0;i--)
				if(p[i]!=0)return 0;
			
			return 1; /*|0|0|0|0|0|0|0|1| big endian*/
		}
	}
	return 0;/*false*/
}


static unsigned int sum(char *s)
{
	int i =0;
	int sum=0;

	for(i=0;i<strlen(s);i++)
		sum=sum+s[i];
	
	return sum;
}

static void check_flag(unsigned int mysum,char *s,FILE *f)
{
	switch(mysum)
	{
		case 400:
		{
			if(strlen(s)==5 && s[0]=='L'&&s[1]=='I'&&s[2]=='N'&&s[3]=='U'&&s[4]=='X')
			{
#if defined(__linux__) || defined(__linux)|| defined(__HOS_LINUX__)
				fputs("#define PROXY_LINUX 1\n",f);
				msg_ok("LINUX flag enable");
#else
				e_warning("OS is not Linux \n",0);
#endif
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag",0);
			}
		}break;
		case 250:
		{
			if(strlen(s)==3 && s[0]=='O'&&s[1]=='S'&&s[2]=='X')
			{
#if defined(__APPLE__) && (_POSIX_VERSION==200112L)	  
				fputs("#define PROXY_OSX 1\n",f);
				msg_ok("APPLE/OSX FLAG");
#else
				e_warning("OS is not APPLE/OSX",0);
#endif
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 238:
		{
			if(strlen(s)==3 && s[0]=='W'&&s[1]=='I'&&s[2]=='N')
			{
#if defined(_WIN32) || defined(__WIN32__) ||defined(_WIN64) || defined(__WIN64__)
				fputs("#define PROXY_WIN 1\n",f);
				e_warning("WIN flag enable, please change your os :P",0);
				fflush(stdout);
#else
				e_warning("WIN , bad flag ",0);
#endif
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag",0);
				fflush(stdout);
			}
		}break;
		case 217:
		{
			if(strlen(s)==3 && s[0]=='B'&&s[1]=='S'&&s[2]=='D')
			{
#if defined(__NetBSD__)||defined( __OpenBSD__)|| defined(__FreeBSD__)
				fputs("#define PROXY_BSD 1\n",f);
				msg_ok("BSD flag enable");
#else
				e_warning("non BSD ",0);
#endif
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 402:
		{
			if(strlen(s)==5 && s[0]=='C'&&s[1]=='R'&&s[2]=='Y'&&s[3]=='P'&&s[4]=='T')
			{
				fputs("#define PROXY_CRYPT 1\n",f);
				msg_ok("CRYPT flag enable");
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 359:
		{
			if(strlen(s)==5 && s[0]=='D'&&s[1]=='E'&&s[2]=='B'&&s[3]=='U'&&s[4]=='G')
			{
				fputs("#define PROXY_DEBUG 1\n",f);
				msg_ok("DEBUG flag enable\n");
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 680:
		{
			if(strlen(s)==9 && s[0]=='L'&&s[1]=='O'&&s[2]=='G'&&s[3]=='_'
				&&s[4]=='D'&&s[5]=='E'&&s[6]=='B'&&s[7]=='U'&&s[8]=='G')
			{
				fputs("#define PROXY_LOG_DEBUG 1\n",f);
				msg_ok("LOG_DEBUG flag enable\n");
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 363:
		{
			if(strlen(s)==5 && s[0]=='S' && s[1]=='E' && s[2]=='N' && s[3]=='D'
			   && s[4]=='A' && strategy==0)
			{
				fputs("#define PROXY_SEND_A 1\n",f);
				msg_ok("PROXY_SEND_A flag enable\n");
				strategy=1;
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 364:
		{
			if(strlen(s)==5 && s[0]=='S' && s[1]=='E' && s[2]=='N' && s[3]=='D'
			   && s[4]=='B' && strategy==0)
			{
				fputs("#define PROXY_SEND_B 1\n",f);
				msg_ok("PROXY_SEND_B flag enable\n");
				strategy=1;
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 365:
		{
			if(strlen(s)==5 && s[0]=='S' && s[1]=='E' && s[2]=='N' && s[3]=='D'
			   && s[4]=='C' && strategy==0)
			{
				fputs("#define PROXY_SEND_C 1\n",f);
				msg_ok("PROXY_SEND_C flag enable\n");
				strategy=1;
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 366:
		{
			if(strlen(s)==5 && s[0]=='S' && s[1]=='E' && s[2]=='N' && s[3]=='D'
			   && s[4]=='D' && strategy==0)
			{
				fputs("#define PROXY_SEND_D 1\n",f);
				msg_ok("PROXY_SEND_D flag enable\n");
				strategy=1;
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 367:
		{
			if(strlen(s)==5 && s[0]=='S' && s[1]=='E' && s[2]=='N' && s[3]=='D'
			   && s[4]=='E' && strategy==0)
			{
				fputs("#define PROXY_SEND_E 1\n",f);
				msg_ok("PROXY_SEND_E flag enable\n");
				strategy=1;
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		case 368:
		{
			if(strlen(s)==5 && s[0]=='S' && s[1]=='E' && s[2]=='N' && s[3]=='D'
			   && s[4]=='F' && strategy==0)
			{
				fputs("#define PROXY_SEND_F 1\n",f);
				msg_ok("PROXY_SEND_F flag enable\n");
				strategy=1;
			}
			else
			{
				e_warning(s,0);
				e_warning("bad flag!!!!",0);
			}
		}break;
		default:
		{
			e_warning(s,0);
			e_warning("bad flag!!!!",0);
		}break;
	}
}

int main(int argc, char *argv[])
{
	int myerror,i=0;
	const int cpu=os_cpu_model();
	FILE *arch_file=NULL;

	msg_help("This file write Proxy_config.h header");
#if	!defined(__STDC__)
	e_fatal("Yor compiler is not conform to ANSI C89",0);
#else
	msg_ok("ANSI C89 conform");
#endif



	arch_file=fopen("Proxy_config.h","w+");
	myerror=errno;
	if(arch_file!=NULL && cpu!=-1 && myerror==0)
	{

#if (_POSIX_VERSION==200112L)	||  _POSIX_C_SOURCE==200112L || defined(__POSIX_VISIBLE) || defined(_POSIX2_VERSION)
		fputs("/*This file is generated by Proxy_configure\n",arch_file);
		fputs("Header store the flag for Proxy_Client Proxy_Server */\n\n",arch_file);
		fputs("#include <stdio.h> \n\n",arch_file);
		fputs("#define PROXY_C89  1 \n",arch_file);
		fputs("#define PROXY_POSIX 200112L  \n",arch_file);
		msg_ok("POSIX 2001 conform");
#else
		fclose(arch_file);
		e_warning("OS is not conform to Posix , program can not run here",myerror);
		e_fatal("OS must be Posix conform ",0);
		return 0;
#endif

		switch(cpu)
		{
			case LP32:
			{
				fputs("#define PROXY_LP32 1\n",arch_file);
				msg_ok("32 bit cpu , LP32 System");
			}break;
			case LP64:
			{
				fputs("#define PROXY_LP64 1\n",arch_file);
				msg_ok("64 bit cpu ,LP64 System");
			}break;
			case ILP64:
			{
				fputs("#define PROXY_ILP64 1\n",arch_file);
				msg_ok("64 bit cpu , ILP64 System");
			}break;
			default:
			{
				fputs("#define PROXY_ILP32 1\n",arch_file);
				msg_ok("32 bit cpu,ILP32 System");
			}break;
		}
	}
	else
	{
		if(cpu==-1)
		{
			e_warning("Program can not run on this computer",0);
			e_fatal("cpu is not supported",0);
			return 0;
		}
		fclose(arch_file);
		e_fatal("Error for write file",myerror);
		return 0;
	}
	
	if(argc>1)
	{
		for(i=1;i<argc;i++)
		{
			check_flag(sum(argv[i]),argv[i],arch_file);
		}
	}
	
	if(is_big())
	{
		fputs("#define PROXY_BIG 1\n",arch_file);
		msg_ok("Found Big Endian cpu");
	}


	if (strategy==0)
		e_fatal("you must select one sending strategy",0);
	
    fputs("typedef unsigned char byte;\n",arch_file);
	fclose(arch_file);
	return 0;
}
#else

#define CONFIGURE_ERROR 1

#endif

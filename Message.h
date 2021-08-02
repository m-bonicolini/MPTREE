/********************************************
 *            message.h
 ********************************************/

#ifndef MESSAGE_H

#define MESSAGE_H 

#include "Proxy_config.h"
#include <stdio.h>
#include <errno.h>



/*Define shell colors with escape sequence*/
#define NORMAL			"\033[0m"
#define RED 				"\033[01;39;31m"
#define BLUE				"\033[01;39;34m"
#define GREEN   			"\033[01;39;32m"
#define YELLOW 			"\033[01;39;33m"
#define MAGENTA		"\033[01;39;35m"
#define CYAN				"\033[01;39s;36m"
#define CLEAR_SCR    "\033[2J"

extern void e_warning(const char*msg,int myerror);/*print warning error*/
extern void e_fatal(const char*msg,int myerror);/* print error message and force quit*/
extern void msg_ok(const char* msg);
extern void msg_help(const char* msg);
extern void msg_tutorial(const char*msg);

#endif

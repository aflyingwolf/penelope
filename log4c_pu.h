#ifndef LOG4C_H_INCLUDED
#define LOG4C_H_INCLUDED

#include <string.h>
#include <stdlib.h>
#include "log4c.h"
#define PU_LOG_CATEGORY_NAME "transmit"
#define PU_LOG_PRIORITY LOG4C_PRIORITY_WARN

//1.LOG4C_PRIORITY_ERROR
//2.LOG4C_PRIORITY_WARN
//3.LOG4C_PRIORITY_NOTICE
//4.LOG4C_PRIORITY_DEBUG
//5.LOG4C_PRIORITY_TRACE

int log4c_pu_init();
void log_message(char* file, int line, const char* func,const char* a_format, ...);
int log4c_pu_fini();
#define LOG(fmt,args...) log_message(__FILE__, __LINE__, __FUNCTION__,fmt ,## args);

#endif // LOG4C_H_INCLUDED

#include "log4c_pu.h"

const char *format = "[%10s][%4d][%10s()]: ";

log4c_category_t* pu_category = NULL;

int log4c_pu_init()
{
	if (log4c_init() == 1)
	{
		return 1;
	}
	pu_category = log4c_category_get(PU_LOG_CATEGORY_NAME);
	return 0 ;
}

void log_message(char* file, int line, const char* func,const char* a_format, ...)
{
	char *file_info;
	char *new_format;
	int info_len;
	int new_format_len;
	va_list va;

	info_len = strlen(format) + 200;
	file_info = (char *) malloc(info_len);
	sprintf(file_info, format,file, line,func );

	new_format_len = strlen(file_info) + strlen(a_format) + 500;
	new_format = (char *) malloc(new_format_len);
	sprintf(new_format, "%s%s", file_info, a_format);

	va_start(va, a_format);
	log4c_category_vlog(pu_category, PU_LOG_PRIORITY, new_format, va);
	va_end(va);

	free(file_info);
	free(new_format);
}

int log4c_pu_fini()
{
	return (log4c_fini());
}

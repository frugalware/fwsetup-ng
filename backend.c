#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "backend.h"

static FILE *out = 0;
static char *text = 0;
static size_t size = 0;
static bool memory = false;
static long errors = 0;

extern int backend_set_log_to_file(const char *path)
{
	// Refuse to change output target more than once.
	if(out != 0)
	{
		errno = EINVAL;

		return -1;
	}

	out = fopen(path,"wb");

	if(out == 0)
		return -1;

	return 0;
}

extern int backend_set_log_to_memory(void)
{
	// Refuse to change output target more than once.
	if(out != 0)
	{
		errno = EINVAL;

		return -1;
	}

	out = open_memstream(&text,&size);

	if(out == 0)
		return -1;

	memory = true;

	return 0;
}

extern int backend_log(const char *format,...)
{
	va_list args;
	int rv = 0;

	if(out == 0 || format == 0)
	{
		errno = EINVAL;

		return -1;
	}

	va_start(args,format);

	if(vfprintf(out,format,args) < 0 || fflush(out) == EOF)
	{
		++errors;

		rv = -1;
	}

	va_end(args);

	return rv;
}

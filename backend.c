#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <parted.h>
#include "backend.h"

static void setup_stderr(void)
{
	static int setup = 0;

	if(setup == 0)
	{
		stderr = freopen("setup.log","wb",stderr);

		if(!stderr)
			abort();

		setup = 1;
	}
}

extern void eprintf(const char *s,...)
{
	assert(s != 0);

	va_list args;

	setup_stderr();

	va_start(args,s);

	vfprintf(stderr,s,args);

	va_end(args);
}

extern void aprintf(const char *s,...)
{
	assert(s != 0);

	va_list args;

	setup_stderr();

	va_start(args,s);

	vfprintf(stderr,s,args);

	va_end(args);

	abort();
}

extern void *allocate(size_t n)
{
	assert(n > 0);

	void *p = malloc(n);

	if(p == 0)
		aprintf("Failed to allocate %zu bytes of memory.\n",n);

	return p;
}

extern void *reallocate(void *p,size_t n)
{
	assert(p != 0);
	assert(n > 0);

	p = realloc(p,n);

	if(p == 0)
		aprintf("Failed to reallocate to %zu bytes of memory.\n",n);

	return p;
}

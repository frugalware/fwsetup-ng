#include "utility.h"

extern void *alloc(size_t n)
{
	assert(n > 0);

	void *p = malloc(n);

	if(p == 0)
		abort();

	return p;
}

extern void *alloc0(size_t n)
{
	assert(n > 0);

	void *p = malloc(n);

	if(p == 0)
		abort();

	memset(p,0,n);

	return p;
}

extern void *redim(void *p,size_t n)
{
	assert(p != 0);
	assert(n > 0);

	p = realloc(p,n);

	if(p == 0)
		abort();

	return p;
}

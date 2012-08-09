#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define talloc(T,N)  ((T*)alloc(sizeof(T)*(N)))
#define talloc0(T,N) ((T*)alloc0(sizeof(T)*(N)))

extern void *alloc(size_t);
extern void *alloc0(size_t);
extern void *redim(void *,size_t);

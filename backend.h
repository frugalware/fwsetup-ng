#pragma once

#include <stddef.h>

extern void eprintf(const char *s,...);
extern void aprintf(const char *s,...);
extern void *allocate(size_t n);
extern void *reallocate(void *p,size_t n);

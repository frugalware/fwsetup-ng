#include "fwsetup.h"

extern void eprintf(const char *s,...)
{
  assert(s != 0);

  va_list args;

  va_start(args,s);

  vfprintf(stderr,s,args);

  va_end(args);
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

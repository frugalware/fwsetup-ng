#include "fwsetup.h"

extern void eprintf(const char *s,...)
{
  assert(s != 0);

  va_list args;

  va_start(args,s);

  vfprintf(stderr,s,args);

  va_end(args);
}

#ifdef NEWT
extern bool get_text_size(const char *text,int *width,int *height)
{
  assert(text != 0);
  assert(width != 0);
  assert(height != 0);

  wchar_t wc = 0;
  size_t n = 0;
  size_t len = strlen(text);
  mbstate_t mbs;
  int cw = 0;
  int w = 0;
  int h = 0;
  int i = 0;

  memzero(&mbs,sizeof(mbstate_t));

  while(true)
  {
    n = mbrtowc(&wc,text,len,&mbs);

    if(n == (size_t) -1 || n == (size_t) -2)
      return false;

    if(n == 0)
      break;

    switch(wc)
    {
      case L'\n':
        if(cw > w)
          w = cw;
        cw = 0;
        ++h;
        break;

      case L'\t':
        cw += 8;
        break;

      default:
        i = wcwidth(wc);
        if(i > 0)
          cw += i;
        break;
    }

    text += n;

    len -= n;
  }

  if(w == 0 && cw > 0)
    w = cw;
  else if(w == 0)
    w = 1;

  if(h == 0)
    h = 1;

  *width = w;

  *height = h;

  return true;
}
#endif

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

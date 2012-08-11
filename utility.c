#include "fwsetup.h"

struct list
{
  struct list *prev;
  struct list *next;
};

static FILE *redirect_std_stream(FILE *oldfp,int oldfd)
{
  assert(oldfp != 0);
  assert(oldfd != -1);

  int newfd = fileno(oldfp);
  FILE *newfp = 0;

  fclose(oldfp);

  close(newfd);

  dup2(oldfd,newfd);

  newfp = fdopen(newfd,"wb");

  setbuf(newfp,0);

  return newfp;
}

extern void eprintf(const char *s,...)
{
  assert(s != 0);

  va_list args;
  static bool prepared = false;

  if(!prepared)
  {
    int fd = open(LOGFILE,O_CREAT|O_TRUNC|O_WRONLY,0644);

    if(fd == -1)
      return;

    stderr = redirect_std_stream(stderr,fd);

#ifndef NEWT
    stdout = redirect_std_stream(stdout,fd);
#endif

    prepared = true;
  }

  va_start(args,s);

  vfprintf(stderr,s,args);

  va_end(args);
}

extern void *list_append(void *list,size_t n)
{
  assert(n > sizeof(struct list));

  struct list *a = list;
  struct list *b = malloc(n);

  if(a == 0)
  {
    b->prev = 0;

    b->next = 0;
  }
  else
  {
    a->next = b;

    b->prev = a;

    b->next = 0;
  }

  return b;
}

extern void *list_find_start(void *list)
{
  assert(list != 0);

  struct list *p = list;

  while(p->prev)
    p = p->prev;

  return p;
}

extern void *list_find_end(void *list)
{
  assert(list != 0);

  struct list *p = list;

  while(p->next)
    p = p->next;

  return p;
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

extern bool get_button_size(const char *text,int *width,int *height)
{
  assert(text != 0);
  assert(width != 0);
  assert(height != 0);

  if(!get_text_size(text,width,height))
    return false;

  *width += 5;

  *height += 3;

  return true;
}
#endif

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

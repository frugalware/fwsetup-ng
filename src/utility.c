#include "local.h"

extern bool mkdir_recurse(const char *path)
{
  char buf[PATH_MAX] = {0};
  char *s = buf + 1;

  if(path == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  snprintf(buf,PATH_MAX,"%s",path);

  while((s = strchr(s,'/')) != 0)
  {
    *s = 0;

    if(mkdir(buf,0755) == -1 && errno != EEXIST)
    {
      fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
      return false;
    }
    
    *s = '/';
    
    ++s;
  }
  
  if(mkdir(buf,0755) == -1 && errno != EEXIST)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  return true;  
}

extern bool size_to_string(char *s,size_t n,long long size)
{
  long long divisor = 0;
  const char *suffix = 0;

  if(s == 0 || n == 0 || size < 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(size >= TEBIBYTE)
  {
    divisor = TEBIBYTE;
    suffix = "TiB";
  }
  else if(size >= GIBIBYTE)
  {
    divisor = GIBIBYTE;
    suffix = "GiB";
  }
  else if(size >= MEBIBYTE)
  {
    divisor = MEBIBYTE;
    suffix = "MiB";
  }
  else if(size >= KIBIBYTE)
  {
    divisor = KIBIBYTE;
    suffix = "KiB";
  }
  else
  {
    divisor = 1;
    suffix = "BiB";
  }

  snprintf(s,n,"%.1f%s",(double) size / divisor,suffix);
  
  return true;
}

extern int get_text_screen_width(const char *s)
{
  wchar_t wc = 0;
  size_t n = 0;
  size_t len = 0;
  mbstate_t mbs = {0};
  int w = 0;
  int i = 0;
  
  if(s == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return -1;
  }
  
  len = strlen(s);
  
  while(true)
  {
    n = mbrtowc(&wc,s,len,&mbs);
    
    if(n == (size_t) -1 || n == (size_t) -2)
    {
      fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
      return -1;
    }
    
    if(n == 0 || wc == L'\n')
      break;
    
    switch(wc)
    {
      case L'\t':
        w += 8;
        break;
      
      default:
        if((i = wcwidth(wc)) > 0)
          w += i;
        break;
    }
    
    s += n;
    
    len -= n;
  }
  
  return w;
}

extern bool get_text_screen_size(const char *text,int *width,int *height)
{
  char *s = text;
  int cw = 0;
  int w = 0;
  int h = 0;

  if(text == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  while(true)
  {
    cw = get_text_screen_length(s);
    
    if(cw == -1)
      return false;
    
    if(w < cw)
      w = cw;
    
    if((s = strchr(s,'\n')) == 0)
      break;
    
    ++h;
    
    ++s;
  }
  
  *width = w;
  
  *height = h;
  
  return true;
}

extern bool get_button_screen_size(const char *text,int *width,int *height)
{
  int w = 0;
  int h = 0;
  
  if(text == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if((w = get_text_screen_length(text)) == -1)
    return false;

  w += 5;
  
  h = 4;
  
  *width = w;
  
  *height = h;
  
  return true;
}

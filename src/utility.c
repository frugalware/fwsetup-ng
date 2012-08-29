#include "local.h"

extern bool mkdir_recurse(const char *path)
{
  char buf[PATH_MAX] = {0};
  char *s = buf;

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

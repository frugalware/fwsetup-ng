#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "Utility.hh"

#define LOGFILE "fwsetup.log"

pid_t execute(const string &cmd)
{
  pid_t pid = -1;

  pid = fork();

  if(pid == 0)
  {
    int fd = open(LOGFILE,O_WRONLY|O_APPEND|O_CREAT,0644);

    if(fd == -1)
      _exit(200);

    dup2(fd,STDOUT_FILENO);

    dup2(fd,STDERR_FILENO);

    close(fd);
    
    execl("/bin/sh","/bin/sh","-c",cmd.c_str(),(void *) 0);
    
    _exit(250);
  }

  return pid;
}

#ifdef NEWT
bool get_text_size(const string &text,int &width,int &height)
{
  wchar_t wc = 0;
  size_t n = 0;
  size_t off = 0;
  mbstate_t mbs;
  int cw = 0;
  int w = 0;
  int h = 0;
  int i = 0;

  memset(&mbs,0,sizeof(mbstate_t));
  
  while(true)
  {
    n = mbrtowc(&wc,text.c_str()+off,text.length()-off,&mbs);
    
    if(n == (size_t) -1 || n == (size_t) -2)
      return false;
    
    if(n == 0)
      break;
    
    switch(wc)
    {
      case L'\t':
        cw += 8;
        break;
      
      case L'\n':
        if(cw > w)
          w = cw;
        cw = 0;
        ++h;
        break;
      
      default:
        i = wcwidth(wc);
        if(i > 0)
          cw += i;
        break;
    }
    
    off += n;
  }
  
  if(w == 0 && cw > 0)
    w = cw;
  else if(w == 0)
    w = 1;
  
  if(h == 0)
    h = 1;
  
  width = w;
  
  height = h;
  
  return true;
}
#endif

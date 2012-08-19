#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sstream>
#include "Utility.hh"

#define LOGFILE "fwsetup.log"

using std::stringstream;

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

unsigned long long string_to_size(const string &text)
{
  unsigned long long n = 0;
  char *p = 0;
  string suffix;

  errno = 0;

  n = strtoull(text.c_str(),&p,10);

  if(errno != 0)
    return 0;

  if(n == 0)
  {
    errno = EINVAL;
    return 0;
  }

  suffix = p;

  if(suffix.empty() || suffix == "B" || suffix == "BiB")
    n *= 1;
  else if(suffix == "K" || suffix == "KiB")
    n *= KIBIBYTE;
  else if(suffix == "M" || suffix == "MiB")
    n *= MEBIBYTE;
  else if(suffix == "G" || suffix == "GiB")
    n *= GIBIBYTE;
  else if(suffix == "T" || suffix == "TiB")
    n *= TEBIBYTE;
  else
  {
    errno = EINVAL;
    return 0;
  }

  return n;
}

string size_to_string(unsigned long long n)
{
  unsigned long long divisor = 0;
  const char *suffix = 0;
  stringstream buf;

  if(n >= TEBIBYTE)
  {
    divisor = TEBIBYTE;
    suffix = "TiB";
  }
  else if(n >= GIBIBYTE)
  {
    divisor = GIBIBYTE;
    suffix = "GiB";
  }
  else if(n >= MEBIBYTE)
  {
    divisor = MEBIBYTE;
    suffix = "MiB";
  }
  else if(n >= KIBIBYTE)
  {
    divisor = KIBIBYTE;
    suffix = "KiB";
  }
  else
  {
    divisor = 1;
    suffix = "BiB";
  }

  buf << (long double) n / divisor << suffix;

  return buf.str();
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
        if((i = wcwidth(wc)) > 0)
          cw += i;
        break;
    }
    
    off += n;
  }
  
  if(w == 0)
    w = (cw > 0) ? cw : 1;
  
  if(h == 0)
    h = 1;
  
  width = w;
  
  height = h;
  
  return true;
}

bool get_button_size(const string &text,int &width,int &height)
{
  if(!get_text_size(text,width,height))
    return false;

  width += 5;

  height += 3;

  return true;
}
#endif

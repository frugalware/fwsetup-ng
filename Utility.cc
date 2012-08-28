#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sstream>
#include "Utility.hh"

#define LOGFILE "fwsetup.log"

using std::stringstream;
using std::ios;
using std::endl;

ofstream logfile(LOGFILE,ios::app);

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

bool zapLabel(const string &path)
{
  string cmd;
  pid_t pid = -1;
  int status = 0;

  cmd = "sgdisk --zap-all " + path;

  logfile << "Executing command: " << cmd << endl;

  if((pid == execute(cmd)) == -1 || waitpid(pid,&status,0) == -1)
  {
    logfile << __func__ << ": " << strerror(errno) << endl;
    return false;
  }

  if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
  {
    logfile << "Command did not exit normally: " << cmd << endl;
    return false;
  }

  logfile << "Finished executing command: " << cmd << endl;

  return true;
}

unsigned long long stringToSize(const string &text)
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

string sizeToString(unsigned long long n)
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

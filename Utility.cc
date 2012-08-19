#include <fcntl.h>
#include <unistd.h>
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

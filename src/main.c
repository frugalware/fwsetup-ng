#include "local.h"

FILE *logfile = 0;

extern int main(int argc,char **argv)
{
  logfile = fopen("fwsetup.log","w");

  if(logfile == 0)
  {
    perror("main");

    return 1;
  }

  return ui_main(argc,argv);
}

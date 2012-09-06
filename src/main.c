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

  setbuf(logfile,0);

  return ui_main(argc,argv);
}

struct global g =
{
  .netinstall = true
};

struct module *modules[] =
{
  &install_module,
  0
};

#include "local.h"

FILE *logfile = 0;

extern int main(int argc,char **argv)
{
  int code = 0;

  logfile = fopen("fwsetup.log","w");

  if(logfile == 0)
  {
    perror("main");

    return 1;
  }

  setbuf(logfile,0);

  parted_initialize();

  code = ui_main(argc,argv);

  fclose(logfile);

  logfile = 0;

  return code;
}

static struct global local =
{
  .netinstall = true
};

struct global *g = &local;

struct module *modules[] =
{
  &install_module,
  &postconfig_module,
  0
};

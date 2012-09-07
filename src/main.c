#include "local.h"

FILE *logfile = 0;

extern int main(int argc,char **argv)
{
  int code = EXIT_SUCCESS;

  logfile = fopen("fwsetup.log","w");

  if(logfile == 0)
  {
    perror("main");

    return 1;
  }

  setbuf(logfile,0);

  code = ui_main(argc,argv);

  fclose(logfile);

  logfile = 0;

  return code;
}

struct global g =
{
  .netinstall = true
};

struct module *modules[] =
{
  &install_module,
  &postconfig_module,
  0
};

#include "local.h"

FILE *logfile = 0;

static PedExceptionOption libparted_exception_callback(PedException *ex)
{
  fprintf(logfile,"libparted: %s %s\n",ped_exception_get_type_string(ex->type),ex->message);

  return PED_EXCEPTION_IGNORE;
}

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

  ped_exception_set_handler(libparted_exception_callback);

  ped_unit_set_default(PED_UNIT_SECTOR);

  g->doslabel = ped_disk_type_get("msdos");

  g->gptlabel = ped_disk_type_get("gpt");

  code = ui_main(argc,argv);

  fclose(logfile);

  logfile = 0;

  return code;
}

static struct global local =
{
  .doslabel = 0,
  .gptlabel = 0,
  .netinstall = true
};

struct global *g = &local;

struct module *modules[] =
{
  &install_module,
  &postconfig_module,
  0
};

#include <newt.h>
#include "local.h"

extern int ui_main(int argc,char **argv)
{
  int w = 0;
  int h = 0;

  if(newtInit() != 0)
  {
    printf(_("Could not initialize the NEWT user interface.\n"));
    return 1;
  }

  newtGetScreenSize(&w,&h);

  if(w < 80 || h < 24)
  {
    printf(_("We require a terminal of 80x24 or greater to use the NEWT user interface.\n"));
    newtFinished();
    return 1;
  }  

  newtCls();

  newtFinished();

  return 0;
}

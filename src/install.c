#include <pacman.h>
#include "local.h"

static bool install_setup(void)
{
  if(!mkdir_recurse(INSTALL_ROOT "/var/cache/pacman-g2/pkg"))
    return false;
  
  if(!mkdir_recurse(INSTALL_ROOT "/var/cache/pacman-g2/src"))
    return false;

  if(!mkdir_recurse(INSTALL_ROOT "/var/lib/pacman-g2/local"))
    return false;

  if(!mkdir_recurse(INSTALL_ROOT "/var/log"))
    return false;

  if(pacman_initialize(INSTALL_ROOT) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  return true;
}

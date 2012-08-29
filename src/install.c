#include <pacman.h>
#include "local.h"

static PM_DB **databases = 0;

static void install_database_callback(const char *name,PM_DB *db)
{
  static size_t n = 1;

  if(name == 0 || db == 0)
    return;

  databases = realloc(databases,sizeof(PM_DB *) * (n + 1));

  databases[n - 1] = db;

  databases[n] = 0;

  ++n;
}

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

  if(pacman_parse_config("/etc/pacman-g2.conf",install_database_callback,"") == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  return true;
}

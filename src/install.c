#include <pacman.h>
#include "local.h"

static PM_DB **databases = 0;
static size_t databases_size = 1;
static char dl_filename[PM_DLFNM_LEN+1] = {0};
static int dl_offset = 0;
static struct timeval dl_time0 = {0};
static struct timeval dl_time1 = {0};
static float dl_rate = 0;
static int dl_xfered1 = 0;
static unsigned char dl_eta_h = 0;
static unsigned char dl_eta_m = 0;
static unsigned char dl_eta_s = 0;
static int dl_remain = 0;
static int dl_total = 0;

static void install_database_callback(const char *name,PM_DB *db)
{
  if(name == 0 || db == 0)
    return;

  databases = realloc(databases,sizeof(PM_DB *) * (databases_size + 1));

  databases[databases_size - 1] = db;

  databases[databases_size] = 0;

  ++databases_size;
}

static int install_download_callback(PM_NETBUF *ctl,int xfered,void *arg)
{
  return 1;
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

  if(pacman_set_option(PM_OPT_DLCB,(long) install_download_callback) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLFNM,(long) dl_filename) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLOFFSET,(long) &dl_offset) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLT0,(long) &dl_time0) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLT,(long) &dl_time1) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLRATE,(long) &dl_rate) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLXFERED1,(long) &dl_xfered1) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLETA_H,(long) &dl_eta_h) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLETA_M,(long) &dl_eta_m) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLETA_S,(long) &dl_eta_s) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLREMAIN,(long) &dl_remain) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_set_option(PM_OPT_DLHOWMANY,(long) &dl_total) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  return true;
}

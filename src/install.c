#include <pacman.h>
#include "local.h"

struct dldata
{
  int amount;
  int total;
  int percent;
  char percent_text[5];
  float timediff;
  char rate_text[47];
};

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

static int install_download_callback(PM_NETBUF *ctl,int dl_xfered0,void *arg)
{
  struct dldata dl = {0};

  dl.amount = dl_xfered0 + dl_offset;

  dl.total = * (int *) arg;

  dl.percent = (float) dl.amount / dl.total * 100;

  snprintf(dl.percent_text,5,"%d%%",dl.percent);

  gettimeofday(&dl_time1,0);

  dl.timediff = (dl_time1.tv_sec - dl_time0.tv_sec) + (float) (dl_time1.tv_usec - dl_time0.tv_usec) / 1000000;

  if(dl.amount == dl.total)
  {
    dl_rate = dl_xfered0 / (dl.timediff * KIBIBYTE);
    dl_eta_h = (int) dl.timediff / 3600;
    dl_eta_m = (int) dl.timediff % 3600 / 60;
    dl_eta_s = (int) dl.timediff % 3600 % 60;
  }
  else if(dl.timediff > 1.0)
  {
    dl_rate = (dl_xfered0 - dl_xfered1) / (dl.timediff * KIBIBYTE);
    dl_xfered1 = dl_xfered0;
    gettimeofday(&dl_time0,0);
    dl_eta_h = (int) ((dl.total - dl.amount) / (dl_rate * KIBIBYTE)) / 3600;
    dl_eta_m = (int) ((dl.total - dl.amount) / (dl_rate * KIBIBYTE)) % 3600 / 60;
    dl_eta_s = (int) ((dl.total - dl.amount) / (dl_rate * KIBIBYTE)) % 3600 % 60;
  }

  snprintf(dl.rate_text,47,"%.1fKiB/s",dl_rate);

  return 1;
}

static bool install_setup(void)
{
  char path[PATH_MAX] = {0};
  const char *dbdir = 0;
  const char *cachedir = 0;
  const char *hooksdir = 0;

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

  if(pacman_get_option(PM_OPT_DBPATH,(long *) &dbdir) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_get_option(PM_OPT_CACHEDIR,(long *) &cachedir) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  if(pacman_get_option(PM_OPT_HOOKSDIR,(long *) &hooksdir) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
    return false;
  }

  snprintf(path,PATH_MAX,"%s/%s",INSTALL_ROOT,dbdir);

  if(!mkdir_recurse(path))
    return false;

  snprintf(path,PATH_MAX,"%s/%s",INSTALL_ROOT,cachedir);

  if(!mkdir_recurse(path))
    return false;

  snprintf(path,PATH_MAX,"%s/%s",INSTALL_ROOT,hooksdir);

  if(!mkdir_recurse(path))
    return false;

  return true;
}

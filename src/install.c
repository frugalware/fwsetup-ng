#include <pacman.h>
#include "local.h"

struct dltext
{
  char eta[9];
  char rate[47];
  char size[20];
  char size_perc[5];
  char pkg[12];
  char pkg_perc[5];
  char *file;
};

typedef int (*install_download_callback_wrapper) (const struct dltext *text);

static PM_DB **databases = 0;
static size_t databases_size = 1;
static char dl_filename[PM_DLFNM_LEN+1] = {0};
static int dl_offset = 0;
static struct timeval dl_time0 = {0};
static struct timeval dl_time1 = {0};
static float dl_rate = 0;
static int dl_xfered1 = 0;
static unsigned int dl_eta_h = 0;
static unsigned int dl_eta_m = 0;
static unsigned int dl_eta_s = 0;
static int dl_remain = 0;
static int dl_howmany = 0;
static install_download_callback_wrapper dl_callback;

static void install_database_callback(const char *name,PM_DB *db)
{
  databases = realloc(databases,sizeof(PM_DB *) * (databases_size + 1));

  databases[databases_size - 1] = db;

  databases[databases_size] = 0;

  ++databases_size;
}

static int install_download_callback(PM_NETBUF *ctl,int dl_xfered0,void *arg)
{
  int dl_amount = 0;
  int dl_total = 0;
  int dl_percent = 0;
  float dl_timediff = 0;
  struct timeval dl_time2 = {0};
  struct dltext text = {{0}, {0}, {0}, {0}, {0}, {0}, 0};
  char *s = 0;

  if(dl_callback == 0)
    return 1;

  dl_amount = dl_xfered0 + dl_offset;

  dl_total = * (int *) arg;

  dl_percent = (float) dl_amount / dl_total * 100;

  gettimeofday(&dl_time2,0);

  if(dl_amount == dl_total)
    dl_time1 = dl_time0;

  dl_timediff = (dl_time2.tv_sec - dl_time1.tv_sec) + (float) (dl_time2.tv_usec - dl_time1.tv_usec) / 1000000;

  if(dl_amount == dl_total)
  {
    dl_rate = dl_xfered0 / (dl_timediff * KIBIBYTE);
    dl_eta_h = (int) dl_timediff / 3600;
    dl_eta_m = (int) dl_timediff % 3600 / 60;
    dl_eta_s = (int) dl_timediff % 3600 % 60;
  }
  else if(dl_timediff > 1.0)
  {
    dl_rate = (dl_xfered0 - dl_xfered1) / (dl_timediff * KIBIBYTE);
    dl_xfered1 = dl_xfered0;
    gettimeofday(&dl_time1,0);
    dl_eta_h = (int) ((dl_total - dl_amount) / (dl_rate * KIBIBYTE)) / 3600;
    dl_eta_m = (int) ((dl_total - dl_amount) / (dl_rate * KIBIBYTE)) % 3600 / 60;
    dl_eta_s = (int) ((dl_total - dl_amount) / (dl_rate * KIBIBYTE)) % 3600 % 60;
  }

  snprintf(text.eta,9,"%u:%u:%u",dl_eta_h,dl_eta_m,dl_eta_s);

  if(dl_rate > KIBIBYTE)
    snprintf(text.rate,47,"%.0fKiB/s",dl_rate);
  else
    snprintf(text.rate,47,"%.1fKiB/s",dl_rate);

  size_to_string(text.size,20,dl_amount);

  snprintf(text.size+strlen(text.size),20-strlen(text.size),"/");

  size_to_string(text.size+strlen(text.size),20-strlen(text.size),dl_total);

  snprintf(text.size_perc,5,"%d%%",dl_percent);

  snprintf(text.pkg,12,"%d/%d",dl_remain,dl_howmany);

  snprintf(text.pkg_perc,5,"%d%%",(int) (float) dl_remain / dl_howmany * 100);

  if((s = strchr(dl_filename,' ')) != 0)
    *s = 0;

  text.file = dl_filename;

  return dl_callback(&text);
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

  if(pacman_set_option(PM_OPT_DLHOWMANY,(long) &dl_howmany) == -1)
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

static bool install_databases_update(void)
{
  size_t i = 0;
  
  while(i < databases_size)
  {
    if(pacman_db_update(1,databases[i]) == -1)
    {
      fprintf(logfile,"%s: %s\n",__func__,pacman_strerror(pm_errno));
      return false;  
    }
    
    ++i;
  }
  
  return true;
}

#include <pacman.h>
#include "local.h"

static char **databases_names = 0;
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

static void install_database_callback(const char *name,PM_DB *db)
{
  databases_names = realloc(databases_names,sizeof(char *) * (databases_size + 1));

  databases_names[databases_size - 1] = strdup(name);

  databases_names[databases_size] = 0;

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
  struct dldata data = {{0}, {0}, {0}, {0}, 0, {0}, {0}, 0, 0};
  char *s = 0;

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

  snprintf(data.eta,9,"%u:%u:%u",dl_eta_h,dl_eta_m,dl_eta_s);

  if(dl_rate > KIBIBYTE)
    snprintf(data.rate,47,"%.0fKiB/s",dl_rate);
  else
    snprintf(data.rate,47,"%.1fKiB/s",dl_rate);

  size_to_string(data.size,20,dl_amount);

  snprintf(data.size+strlen(data.size),20-strlen(data.size),"/");

  size_to_string(data.size+strlen(data.size),20-strlen(data.size),dl_total);

  snprintf(data.size_perc,5,"%d%%",dl_percent);

  data.size_perc_int = dl_percent;

  snprintf(data.pkg,12,"%d/%d",dl_remain,dl_howmany);

  snprintf(data.pkg_perc,5,"%d%%",(int) (float) dl_remain / dl_howmany * 100);

  data.pkg_perc_int = (int) (float) dl_remain / dl_howmany * 100;

  if((s = strchr(dl_filename,' ')) != 0)
    *s = 0;

  data.file = dl_filename;

  ui_dialog_progress_install("Downloading...",&data);

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

static int install_run(void)
{
  struct install groups[] =
  {
    {              "apps", false },
    {        "apps-extra", false },
    {              "base", false },
    {        "base-extra", false },
    {             "devel", false },
    {       "devel-extra", false },
    {             "gnome", false },
    {       "gnome-extra", false },
    {               "kde", false },
    {         "kde-extra", false },
    {               "lib", false },
    {         "lib-extra", false },
    {        "multimedia", false },
    {  "multimedia-extra", false },
    {           "network", false },
    {     "network-extra", false },
    {               "x11", false },
    {         "x11-extra", false },
    {             "xapps", false },
    {       "xapps-extra", false },
    {             "xfce4", false },
    {       "xfce4-extra", false },
    {              "xlib", false },
    {        "xlib-extra", false },
    {       "xmultimedia", false },
    { "xmultimedia-extra", false },
    {                   0, false }
  };

  return 0;
}

static void install_reset(void)
{
}

struct module install_module =
{
  install_run,
  install_reset,
  __FILE__
};

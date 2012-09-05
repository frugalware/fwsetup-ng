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
  
  while(databases[i] != 0)
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

static bool install_groups_get(struct install **groups)
{
  size_t matches = 0; 
  struct install *grps = 0;
  size_t j = 0;

  for( size_t i = 0 ; databases[i] != 0 ; ++i )
  {
    const char *s = (const char *) pacman_db_getinfo(databases[i],PM_DB_TREENAME);
      
    if(s == 0 || (strcmp(s,"frugalware") != 0 && strcmp(s,"frugalware-current") != 0))
      continue;

    PM_LIST *list = pacman_db_getgrpcache(databases[i]);
    
    if(list == 0)
      continue;
    
    for( ; list ; list = pacman_list_next(list) )
    {
      const char *s = (const char *) pacman_list_getdata(list);
      
      if(s == 0)
        continue;
      
      if(strcmp(s,"apps") == 0)
        ++matches;
      else if(strcmp(s,"base") == 0)
        ++matches;
      else if(strcmp(s,"devel") == 0)
        ++matches;
      else if(strcmp(s,"gnome") == 0)
        ++matches;
      else if(strcmp(s,"kde") == 0)
        ++matches;
      else if(strcmp(s,"lib") == 0)
        ++matches;
      else if(strcmp(s,"multimedia") == 0)
        ++matches;
      else if(strcmp(s,"network") == 0)
        ++matches;
      else if(strcmp(s,"x11") == 0)
        ++matches;
      else if(strcmp(s,"xapps") == 0)
        ++matches;
      else if(strcmp(s,"xfce4") == 0)
        ++matches;
      else if(strcmp(s,"xlib") == 0)
        ++matches;
      else if(strcmp(s,"xmultimedia") == 0)
        ++matches;
      else if(strstr(s,"-extra") != 0)
        ++matches;
    }

    break;
  }
  
  if(matches == 0)
    return false;
  
  grps = malloc(sizeof(struct install) * (matches + 1));

  for( size_t i = 0 ; databases[i] != 0 ; ++i )
  {
    const char *s = (const char *) pacman_db_getinfo(databases[i],PM_DB_TREENAME);
      
    if(s == 0 || (strcmp(s,"frugalware") != 0 && strcmp(s,"frugalware-current") != 0))
      continue;

    PM_LIST *list = pacman_db_getgrpcache(databases[i]);
    
    if(list == 0)
      continue;
    
    for( ; list ; list = pacman_list_next(list) )
    {
      const char *s = (const char *) pacman_list_getdata(list);
      bool cache = false;
      
      if(s == 0)
        continue;
      
      if(strcmp(s,"apps") == 0)
        cache = true;
      else if(strcmp(s,"base") == 0)
        cache = true;
      else if(strcmp(s,"devel") == 0)
        cache = true;
      else if(strcmp(s,"gnome") == 0)
        cache = true;
      else if(strcmp(s,"kde") == 0)
        cache = true;
      else if(strcmp(s,"lib") == 0)
        cache = true;
      else if(strcmp(s,"multimedia") == 0)
        cache = true;
      else if(strcmp(s,"network") == 0)
        cache = true;
      else if(strcmp(s,"x11") == 0)
        cache = true;
      else if(strcmp(s,"xapps") == 0)
        cache = true;
      else if(strcmp(s,"xfce4") == 0)
        cache = true;
      else if(strcmp(s,"xlib") == 0)
        cache = true;
      else if(strcmp(s,"xmultimedia") == 0)
        cache = true;
      else if(strstr(s,"-extra") != 0)
        cache = true;
      
      if(cache)
      {
        grps[j].name = strdup(s);
        grps[j].checked = false;
        ++j;
      }
    }

    break;
  }

  grps[j].name = 0;
  
  grps[j].checked = false;
  
  *groups = grps;
  
  return true;
}

static int install_run(void)
{
  struct install *groups = 0;
  int order = 0;

  if(!install_setup())
    return 0;

  if(g.netinstall && !install_databases_update())
    return 0;

  if(!install_groups_get(&groups))
    return 0;

  if((order = ui_window_install(INSTALL_TITLE_TEXT,groups)) == 0)
    return 0;

  return order;
}

static void install_reset(void)
{
  pacman_release();
  
  if(databases_names != 0)
  {
    for( char **s = databases_names ; *s ; ++s )
      free(*s);
  
    free(databases_names);
    
    databases_names = 0;
  }
  
  free(databases);
  
  databases = 0;
  
  databases_size = 1;
  
  memset(dl_filename,0,sizeof(dl_filename));

  dl_offset = 0;
  
  memset(&dl_time0,0,sizeof(dl_time0));
  
  memset(&dl_time1,0,sizeof(dl_time1));

  dl_rate = 0;
  
  dl_xfered1 = 0;

  dl_eta_h = 0;
  
  dl_eta_m = 0;
  
  dl_eta_s = 0;
  
  dl_remain = 0;
  
  dl_howmany = 0;
}

struct module install_module =
{
  install_run,
  install_reset,
  __FILE__
};

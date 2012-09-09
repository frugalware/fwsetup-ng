#include "local.h"

#define TZSEARCHDIR "usr/share/zoneinfo/posix"
#define TZDIR "usr/share/zoneinfo"
#define TZFILE "etc/localtime"
#define TZSEARCHSIZE (sizeof(TZSEARCHDIR)-1)

static size_t tz_size = 0;
static size_t tz_count = 0;
static char **tz_data = 0;

static bool is_root_setup(void)
{
  FILE *file = 0;
  char line[LINE_MAX] = {0};
  char *tmp = 0;
  char *user = 0;
  char *pwd = 0;
  bool result = false;

  file = fopen("etc/shadow","rb");

  if(file == 0)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  while(fgets(line,LINE_MAX,file) != 0)
  {
    tmp = line;

    if((user = strsep(&tmp,":")) == 0)
      continue;
    
    if((pwd = strsep(&tmp,":")) == 0)
      continue;
    
    if(strcmp(user,"root") == 0)
    {
      result = (strlen(pwd) > 1);
      break;
    }
  }
  
  fclose(file);
  
  return result;
}

static bool is_user_setup(void)
{
  FILE *file = 0;
  char line[LINE_MAX] = {0};
  char *tmp = 0;
  char *gid = 0;
  bool result = false;
  
  file = fopen("etc/passwd","rb");
  
  if(file == 0)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;    
  }
  
  while(fgets(line,LINE_MAX,file) != 0)
  {
    tmp = line;

    if(strsep(&tmp,":") == 0)
      continue;
    
    if(strsep(&tmp,":") == 0)
      continue;
    
    if(strsep(&tmp,":") == 0)
      continue;
    
    if((gid = strsep(&tmp,":")) == 0)
      continue;
    
    if(strcmp(gid,"100") == 0)
    {
      result = true;
      break;
    }
  }

  fclose(file);
  
  return result;
}

static bool root_action(struct account *account)
{
  char command[_POSIX_ARG_MAX] = {0};

  if(account == 0 || account->user == 0 || account->password == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  snprintf(command,_POSIX_ARG_MAX,"echo '%s:%s' | chpasswd",account->user,account->password);  
  
  return execute(command,INSTALL_ROOT,0);
}

static bool user_action(struct account *account)
{
  char command[_POSIX_ARG_MAX] = {0};

  if(account == 0 || account->user == 0 || account->password == 0 || account->group == 0 || account->groups == 0 || account->home == 0 || account->shell == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  snprintf(command,_POSIX_ARG_MAX,"useradd -m -c '%s' -g '%s' -G '%s' -d '%s' -s '%s' '%s'",(account->name != 0) ? account->name : "",account->group,account->groups,account->home,account->shell,account->user);

  if(!execute(command,INSTALL_ROOT,0))
    return false;
  
  snprintf(command,_POSIX_ARG_MAX,"echo '%s:%s' | chpasswd",account->user,account->password);
  
  if(!execute(command,INSTALL_ROOT,0))
    return false;
  
  return true;
}

static void account_free(struct account *account)
{
  if(account == 0)
    return;

  free(account->name);

  free(account->user);
  
  free(account->password);
  
  free(account->group);
  
  free(account->groups);
  
  free(account->home);
  
  free(account->shell);
  
  memset(account,0,sizeof(struct account));
}

static int timezone_nftw_callback(const char *path,const struct stat *st,int type,struct FTW *fb)
{
  if(type == FTW_D || type == FTW_DP)
    return 0;

  if(tz_data == 0)
  {
    ++tz_size;
  }
  else
  {
    tz_data[tz_count] = strdup(path + TZSEARCHSIZE + 1);
    ++tz_count;
  }

  return 0;
}

static int timezone_cmp_callback(const void *a,const void *b)
{
  const char *A = *(const char **) a;
  const char *B = *(const char **) b;

  return strcmp(A,B);
}

static bool get_timezone_data(void)
{
  if(nftw(TZSEARCHDIR,timezone_nftw_callback,512,FTW_DEPTH|FTW_PHYS) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  tz_data = malloc0(sizeof(char *) * (tz_size + 1));

  if(nftw(TZSEARCHDIR,timezone_nftw_callback,512,FTW_DEPTH|FTW_PHYS) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  qsort(tz_data,tz_size,sizeof(char *),timezone_cmp_callback);

  return true;
}

static bool time_action(char *zone,bool utc)
{
  char buf[4096] = {0};

  if(unlink(TZFILE) == -1 && errno != ENOENT)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  snprintf(buf,4096,"/" TZDIR "/%s",zone);
  
  if(symlink(buf,TZFILE) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  snprintf(buf,4096,"hwclock --systohc %s",(utc) ? "--utc" : "--localtime");

  if(!execute(buf,INSTALL_ROOT,0))
    return false;

  return true;
}

static bool postconfig_run(void)
{
  struct account account = {0};
  char *zone = 0;
  bool utc = true;
  
  if(chdir(INSTALL_ROOT) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(mount("none",INSTALL_ROOT "/dev","devtmpfs",0,0) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(mount("none",INSTALL_ROOT "/proc","proc",0,0) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(mount("none",INSTALL_ROOT "/sys","sysfs",0,0) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(!is_root_setup() && (!ui_window_root(&account) || !root_action(&account)))
  {
    account_free(&account);
    return false;
  }
  
  account_free(&account);
  
  if(!is_user_setup() && (!ui_window_user(&account) || !user_action(&account)))
  {
    account_free(&account);
    return false;
  }
  
  account_free(&account);

  if(!get_timezone_data() || !ui_window_time(tz_data,&zone,&utc) || !time_action(zone,utc))
    return false;

  if(umount2(INSTALL_ROOT "/dev",MNT_DETACH) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;    
  }
  
  if(umount2(INSTALL_ROOT "/proc",MNT_DETACH) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(umount2(INSTALL_ROOT "/sys",MNT_DETACH) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  return true;
}

static void postconfig_reset(void)
{
  tz_size = 0;

  tz_count = 0;

  if(tz_data != 0)
  {
    for( size_t i = 0 ; tz_data[i] != 0 ; ++i )
      free(tz_data[i]);
    
    free(tz_data);
    
    tz_data = 0;
  }
}

struct module postconfig_module =
{
  postconfig_run,
  postconfig_reset,
  __FILE__
};

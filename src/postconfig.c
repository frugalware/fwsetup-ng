#include "local.h"

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
    if((user = strtok_r(line,":",&tmp)) == 0)
      continue;
    
    if((pwd = strtok_r(0,":",&tmp)) == 0)
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
    if(strtok_r(line,":",&tmp) == 0)
      continue;
    
    if(strtok_r(0,":",&tmp) == 0)
      continue;
    
    if(strtok_r(0,":",&tmp) == 0)
      continue;
    
    if((gid = strtok_r(0,":",&tmp)) == 0)
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

  snprintf(command,_POSIX_ARG_MAX,"useradd -D -m -c '%s' -g '%s' -G '%s' -d '%s' -s '%s' '%s'",(account->name != 0) ? account->name : "",account->group,account->groups,account->home,account->shell,account->user);

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

static bool postconfig_run(void)
{
  struct account account = {0};
  
  if(chdir(INSTALL_ROOT) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(!is_root_setup() && !ui_window_root(&account) && !root_action(&account))
  {
    account_free(&account);
    return false;
  }
  
  account_free(&account);
  
  if(!is_user_setup() && !ui_window_user(&account) && !user_action(&account))
  {
    account_free(&account);
    return false;
  }
  
  account_free(&account);
  
  return true;
}

static void postconfig_reset(void)
{
}

struct module postconfig_module =
{
  postconfig_run,
  postconfig_reset,
  __FILE__
};

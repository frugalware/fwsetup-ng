#include "local.h"

static bool parted_partition_change_active(struct parted *parted,int n,bool state)
{
  PedPartition *part = 0;
  PedPartitionFlag flag = 0;

  if(parted == 0 || parted->device == 0 || parted->constraint == 0 || parted->disk == 0 || n < 1)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if((part = ped_disk_get_partition(parted->disk,n)) == 0)
    return false;

  if(parted->disk->type == g->doslabel)
    flag = PED_PARTITION_BOOT;
  else if(parted->disk->type == g->gptlabel)
    flag = PED_PARTITION_LEGACY_BOOT;
  else
    return false;

  parted->modified = true;

  return ped_partition_set_flag(part,flag,state);
}

extern bool mkdir_recurse(const char *path)
{
  char buf[PATH_MAX] = {0};
  char *s = buf + 1;

  if(path == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  snprintf(buf,PATH_MAX,"%s",path);

  while((s = strchr(s,'/')) != 0)
  {
    *s = 0;

    if(mkdir(buf,0755) == -1 && errno != EEXIST)
    {
      fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
      return false;
    }
    
    *s = '/';
    
    ++s;
  }
  
  if(mkdir(buf,0755) == -1 && errno != EEXIST)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  return true;  
}

extern bool size_to_string(char *s,size_t n,long long size,bool pad)
{
  long long divisor = 0;
  const char *suffix = 0;

  if(s == 0 || n == 0 || size < 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(size >= TEBIBYTE)
  {
    divisor = TEBIBYTE;
    suffix = "TiB";
  }
  else if(size >= GIBIBYTE)
  {
    divisor = GIBIBYTE;
    suffix = "GiB";
  }
  else if(size >= MEBIBYTE)
  {
    divisor = MEBIBYTE;
    suffix = "MiB";
  }
  else if(size >= KIBIBYTE)
  {
    divisor = KIBIBYTE;
    suffix = "KiB";
  }
  else
  {
    divisor = 1;
    suffix = "BiB";
  }

  snprintf(s,n,"%*.1f%s",(pad) ? 6 : 0,(double) size / divisor,suffix);
  
  return true;
}

extern int get_text_length(const char *s)
{
  wchar_t wc = 0;
  size_t n = 0;
  size_t len = 0;
  mbstate_t mbs = {0};
  int l = 0;
  
  if(s == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return -1;
  }
  
  len = strlen(s);
  
  while(true)
  {
    n = mbrtowc(&wc,s,len,&mbs);

    if(n == (size_t) -1 || n == (size_t) -2)
    {
      fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
      return -1;
    }

    if(n == 0)
      break;

    ++l;

    s += n;

    len -= n;
  }
  
  return l;
}

extern bool execute(const char *command,const char *root,pid_t *cpid)
{
  pid_t pid = -1;
  int status = 0;

  if(command == 0 || root == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  fprintf(logfile,_("Attempting to execute command '%s' with root directory '%s'.\n"),command,root);
  
  if((pid = fork()) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(pid == 0)
  {
    int fd = open(LOGFILE,O_WRONLY|O_APPEND|O_CREAT,0644);
    
    if(fd == -1)
      _exit(200);
    
    dup2(fd,STDOUT_FILENO);
    
    dup2(fd,STDERR_FILENO);
    
    close(fd);
    
    if(chroot(root) == -1)
      _exit(210);
      
    if(chdir("/") == -1)
      _exit(220);      
    
    execl("/bin/sh","/bin/sh","-c",command,(void *) 0);
    
    _exit(230);
  }
  
  if(cpid != 0)
  {
    *cpid = pid;
    return true;
  }
  
  if(waitpid(pid,&status,0) == -1 || !WIFEXITED(status))
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  fprintf(logfile,_("Command '%s' which was executed with root directory '%s' has exitted with code '%d'.\n"),command,root,WEXITSTATUS(status));
  
  return (WEXITSTATUS(status) == 0);
}

extern void *malloc0(size_t size)
{
  if(size == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return 0;
  }
  
  return memset(malloc(size),0,size);
}

extern struct parted *parted_open(const char *path)
{
  PedDevice *device = 0;
  PedConstraint *constraint = 0;
  PedDiskType *disktype = 0;
  PedDisk *disk = 0;
  struct parted *parted = 0;

  if(path == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return 0;
  }
  
  if((device = ped_device_get(path)) == 0)
    return 0;
  
  if((constraint = ped_device_get_optimal_aligned_constraint(device)) == 0)
  {
    ped_device_destroy(device);
    return 0;
  }
  
  if((disktype = ped_disk_probe(device)) != 0 && (disktype == g->doslabel || disktype == g->gptlabel))
    disk = ped_disk_new(device);

  parted = malloc0(sizeof(struct parted));

  parted->device = device;

  parted->constraint = constraint;

  parted->disk = disk;

  parted->modified = false;

  return parted;
}

extern bool parted_new_disk_label(struct parted *parted,PedDiskType *type)
{
  if(parted == 0 || parted->device == 0 || parted->constraint == 0 || type == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(parted->disk != 0)
    ped_disk_destroy(parted->disk);

  parted->disk = ped_disk_new_fresh(parted->device,type);
  
  parted->modified = true;
  
  return (parted->disk != 0);
}

extern bool parted_new_partition(struct parted *parted,const char *size)
{
  PedSector sector = 0;
  PedGeometry *range = 0;
  PedPartition *part = 0;
  
  if(parted == 0 || parted->device == 0 || parted->constraint == 0 || parted->disk == 0 || size == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(ped_unit_parse(size,parted->device,&sector,&range) == 0)
    return false;

  if((part = ped_partition_new(parted->disk,PED_PARTITION_NORMAL,0,range->start,range->end)) == 0)
  {
    ped_geometry_destroy(range);
    return false;
  }

  if(ped_disk_add_partition(parted->disk,part,parted->constraint) == 0)
  {
    ped_geometry_destroy(range);
    ped_partition_destroy(part);
    return false;
  }

  parted->modified = true;
  
  ped_geometry_destroy(range);

  return true;
}

extern bool parted_partition_get_active(struct parted *parted,int n)
{
  PedPartition *part = 0;
  PedPartitionFlag flag = 0;

  if(parted == 0 || parted->device == 0 || parted->constraint == 0 || parted->disk == 0 || n < 1)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if((part = ped_disk_get_partition(parted->disk,n)) == 0)
    return false;

  if(parted->disk->type == g->doslabel)
    flag = PED_PARTITION_BOOT;
  else if(parted->disk->type == g->gptlabel)
    flag = PED_PARTITION_LEGACY_BOOT;
  else
    return false;

  return ped_partition_get_flag(part,flag);
}

extern bool parted_partition_set_active(struct parted *parted,int n)
{
  return parted_partition_change_active(parted,n,true);
}

extern bool parted_partition_unset_active(struct parted *parted,int n)
{
  return parted_partition_change_active(parted,n,false);
}

extern bool parted_delete_last_partition(struct parted *parted)
{
  PedPartition *part = 0;
  
  if(parted == 0 || parted->device == 0 || parted->constraint == 0 || parted->disk == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  for( PedPartition *i = 0 ; (i = ped_disk_next_partition(parted->disk,i)) != 0 ; )
  {
    if(i->num > 0 && is_normal_partition(i->type))
      part = i;
  }

  if(part == 0)
    return true;
  
  parted->modified = true;
  
  return (ped_disk_delete_partition(parted->disk,part) != 0);
}

extern void parted_close(struct parted *parted)
{
  if(parted != 0)
  {
    if(parted->disk != 0)
      ped_disk_destroy(parted->disk);

    if(parted->constraint != 0)
      ped_constraint_destroy(parted->constraint);
  
    if(parted->device != 0)
      ped_device_destroy(parted->device);
  
    free(parted);
  }
}

extern int get_text_screen_width(const char *s)
{
  wchar_t wc = 0;
  size_t n = 0;
  size_t len = 0;
  mbstate_t mbs = {0};
  int w = 0;
  int i = 0;
  
  if(s == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return -1;
  }
  
  len = strlen(s);
  
  while(true)
  {
    n = mbrtowc(&wc,s,len,&mbs);
    
    if(n == (size_t) -1 || n == (size_t) -2)
    {
      fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
      return -1;
    }
    
    if(n == 0 || wc == L'\n')
      break;
    
    switch(wc)
    {
      case L'\t':
        w += 8;
        break;
      
      default:
        if((i = wcwidth(wc)) > 0)
          w += i;
        break;
    }
    
    s += n;
    
    len -= n;
  }
  
  return w;
}

extern bool get_text_screen_size(const char *text,int *width,int *height)
{
  const char *s = text;
  int cw = 0;
  int w = 0;
  int h = 0;

  if(text == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  while(true)
  {
    cw = get_text_screen_width(s);
    
    if(cw == -1)
      return false;
    
    if(w < cw)
      w = cw;
    
    if((s = strchr(s,'\n')) == 0)
      break;
    
    ++h;
    
    ++s;
  }
  
  *width = w;
  
  *height = h;
  
  return true;
}

extern bool get_button_screen_size(const char *text,int *width,int *height)
{
  int w = 0;
  int h = 0;
  
  if(text == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if((w = get_text_screen_width(text)) == -1)
    return false;

  w += 5;
  
  h = 4;
  
  *width = w;
  
  *height = h;
  
  return true;
}

extern bool get_label_screen_size(const char *text,int *width,int *height)
{
  int w = 0;
  int h = 0;

  if(text == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if((w = get_text_screen_width(text)) == -1)
    return false;

  h = 1;

  *width = w;

  *height = h;

  return true;
}

extern bool get_checkbox_screen_size(const char *text,int *width,int *height)
{
  int w = 0;
  int h = 0;
  
  if(text == 0 || width == 0 || height == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if((w = get_text_screen_width(text)) == -1)
    return false;
  
  w += 4;
  
  h = 1;
  
  *width = w;
  
  *height = h;
  
  return true; 
}

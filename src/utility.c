#include <blkid.h>
#include <linux/major.h>
#include "local.h"

static inline bool is_ide_disk(const struct stat *st)
{
  switch(major(st->st_rdev))
  {
    case IDE0_MAJOR:
    case IDE1_MAJOR:
    case IDE2_MAJOR:
    case IDE3_MAJOR:
    case IDE4_MAJOR:
    case IDE5_MAJOR:
    case IDE6_MAJOR:
    case IDE7_MAJOR:
    case IDE8_MAJOR:
    case IDE9_MAJOR:
      return true;
    default:
      return false;
  }
}

static inline bool is_scsi_disk(const struct stat *st)
{
  switch(major(st->st_rdev))
  {
    case SCSI_DISK0_MAJOR:
    case SCSI_DISK1_MAJOR:
    case SCSI_DISK2_MAJOR:
    case SCSI_DISK3_MAJOR:
    case SCSI_DISK4_MAJOR:
    case SCSI_DISK5_MAJOR:
    case SCSI_DISK6_MAJOR:
    case SCSI_DISK7_MAJOR:
    case SCSI_DISK8_MAJOR:
    case SCSI_DISK9_MAJOR:
    case SCSI_DISK10_MAJOR:
    case SCSI_DISK11_MAJOR:
    case SCSI_DISK12_MAJOR:
    case SCSI_DISK13_MAJOR:
    case SCSI_DISK14_MAJOR:
    case SCSI_DISK15_MAJOR:
      return true;
    default:
      return false;
  }
}

static inline bool is_virtio_disk(const struct stat *st)
{
  return (major(st->st_rdev) == 253);
}

static inline bool is_raid(const struct stat *st)
{
  return (major(st->st_rdev) == MD_MAJOR);
}

static inline bool is_lvm(const struct stat *st)
{
  return (major(st->st_rdev) == 254);
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

extern struct device *device_read(const char *path)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_topology topology = 0;
  long long sectorsize = 0;
  long long alignment = 0;
  blkid_loff_t size = 0;
  long long sectors = 0;
  struct stat st = {0};
  bool disk = false;
  bool raid = false;
  bool lvm = false;
  blkid_partlist partlist = 0;
  blkid_parttable parttable = 0;
  const char *label = 0;
  bool dos = false;
  bool gpt = false;
  blkid_partition partition = 0;
  void *table[128] = {0};
  size_t n = 0;
  struct device *device = 0;

  if(path == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return 0;
  }

  fprintf(logfile,_("About to read a device from path '%s'.\n"),path);

  if((fd = open(path,O_RDONLY)) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }

  if((probe = blkid_new_probe()) == 0)
  {
    fprintf(logfile,_("Failed to allocate a blkid_probe.\n"));
    goto bail;
  }

  if(blkid_probe_set_device(probe,fd,0,0) == -1)
  {
    fprintf(logfile,_("Failed to set a blkid_probe's device.\n"));
    goto bail;
  }

  if((topology = blkid_probe_get_topology(probe)) == 0)
  {
    fprintf(logfile,_("Failed to get a blkid_probe's topology.\n"));
    goto bail;
  }

  sectorsize = blkid_topology_get_logical_sector_size(topology);

  alignment = MEBIBYTE / sectorsize;
  
  size = blkid_probe_get_size(probe);

  sectors = size / sectorsize;

  if(sectorsize == 0)
  {
    fprintf(logfile,_("Sector size is 0 when it should not be.\n"));
    goto bail;
  }

  if((MEBIBYTE % sectorsize) != 0)
  {
    fprintf(logfile,_("Sector size won't cleanly divide a MEBIBYTE.\n"));
    goto bail;
  }

  if(size <= 0)
  {
    fprintf(logfile,_("The size of this device is less than or equal to 0.\n"));
    goto bail;
  }

  if((size % sectorsize) != 0)
  {
    fprintf(logfile,_("The size of this device is not cleanly divisible by the sector size.\n"));
    goto bail;
  }

  if(fstat(fd,&st) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }

  if(is_ide_disk(&st) || is_scsi_disk(&st) || is_virtio_disk(&st))
    disk = true;
  else if(is_raid(&st))
    raid = true;
  else if(is_lvm(&st))
    lvm = true;
  else
  {
    fprintf(logfile,_("Unrecognized device type.\n"));
    goto bail;
  }

  if(disk)
  {
    if( 
        (partlist = blkid_probe_get_partitions(probe)) != 0    &&
        (parttable = blkid_partlist_get_table(partlist)) != 0  &&
        (label = blkid_parttable_get_type(parttable)) != 0     &&
        (strcmp(label,"dos") == 0 || strcmp(label,"gpt") == 0)
      )
    {
      int i = 0;
      int j = blkid_partlist_numof_partitions(partlist);
      
      dos = (strcmp(label,"dos") == 0);
      
      gpt = (strcmp(label,"gpt") == 0);
      
      if((dos && j > 60) || (gpt && j > 128))
      {
        fprintf(logfile,_("Partition table exceeds entry limits.\n"));
        goto bail;
      }
      
      while(i < j && (partition = blkid_partlist_get_partition(partlist,i)) != 0)
      {
        if(dos)
        {
          struct dospartition *part = malloc0(sizeof(struct dospartition));

          part->number = blkid_partition_get_partno(partition);
          
          part->start = blkid_partition_get_start(partition);
          
          part->end = blkid_partition_get_start(partition) + blkid_partition_get_size(partition) - 1;
          
          part->sectors = blkid_partition_get_size(partition);
          
          part->active = (blkid_partition_get_flags(partition) == 0x80);
          
          part->type = blkid_partition_get_type(partition);
          
          table[i] = part;
        }

        if(gpt)
        {
          struct gptpartition *part = malloc0(sizeof(struct gptpartition));
          const char *type = 0;
          const char *name = 0;
          const char *uuid = 0;
          
          part->number = blkid_partition_get_partno(partition);
          
          part->start = blkid_partition_get_start(partition);
          
          part->end = blkid_partition_get_start(partition) + blkid_partition_get_size(partition) - 1;
          
          part->sectors = blkid_partition_get_size(partition);
          
          part->flags = blkid_partition_get_flags(partition);
          
          type = blkid_partition_get_type_string(partition);
          
          part->type = (type != 0) ? strdup(type) : 0;
          
          name = blkid_partition_get_name(partition);
          
          part->name = (name != 0) ? strdup(name) : 0;
          
          uuid = blkid_partition_get_uuid(partition);
        
          part->uuid = (uuid != 0) ? strdup(uuid) : 0;

          table[i] = part;
        }

        ++i;
      }
      
      n = i;
    }
  }

  device = malloc0(sizeof(struct device));

  device->path = strdup(path);
  
  device->sectorsize = sectorsize;
  
  device->alignment = alignment;
  
  device->sectors = sectors;

  device->disk = disk;
  
  device->raid = raid;
  
  device->lvm = lvm;
  
  device->dos = dos;
  
  device->gpt = gpt;
  
  memcpy(device->table,table,sizeof(table));

  device->size = n;

bail:

  if(fd != -1)
    close(fd);

  if(probe != 0)
    blkid_free_probe(probe);

  return device;
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

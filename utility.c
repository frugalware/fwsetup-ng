#include "fwsetup.h"

#define INVALID_TABLE_TEXT _("This unsupported partition table (%s) cannot be written to %s.\n")

struct list
{
  struct list *prev;
  struct list *next;
};

static FILE *logfile = 0;

static inline bool is_ide_disk(const struct stat *st)
{
  ASSERT_ARGS(st == 0,false);
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
  ASSERT_ARGS(st == 0,false);
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
  ASSERT_ARGS(st == 0,false);
  return (major(st->st_rdev) == VIRTBLK_MAJOR);
}

static inline bool is_mdadm_device(const struct stat *st)
{
  ASSERT_ARGS(st == 0,false);
  return (major(st->st_rdev) == MD_MAJOR);
}

static char *get_gpt_disk_uuid(const char *path)
{
  ASSERT_ARGS(path == 0,0);

  char cmd[_POSIX_ARG_MAX] = {0};
  FILE *inpipe = 0;
  char line[LINE_MAX] = {0};
  char *p = 0;

  snprintf(cmd,sizeof(cmd),"sgdisk -p %s 2> /dev/null | sed -rn 's|^Disk identifier \\(GUID\\): ([0-9a-zA-Z-]+)$|\\1|p' 2> /dev/null",path);

  inpipe = popen(cmd,"r");

  if(inpipe == 0)
  {
    eprintf("%s: %s",__func__,strerror(errno));
    return 0;
  }

  if(fgets(line,sizeof(line),inpipe) == 0)
  {
    pclose(inpipe);
    eprintf("%s: %s",__func__,strerror(errno));
    return 0;
  }

  if(pclose(inpipe) == -1)
  {
    eprintf("%s: %s",__func__,strerror(errno));
    return 0;
  }

  p = strchr(line,'\n');

  if(p != 0)
    *p = 0;

  return strdup(line);
}

extern void *malloc0(size_t n)
{
  ASSERT_ARGS(n == 0,0);

  void *p = malloc(n);

  memzero(p,n);

  return p;
}

extern pid_t execute(const char *cmd)
{
  ASSERT_ARGS(cmd == 0,-1);
  
  pid_t pid = -1;
  int fd = -1;

  pid = fork();

  if(pid == 0)
  {
    fd = open(LOGFILE,O_WRONLY|O_APPEND|O_CREAT,0644);

    if(fd == -1)
      _exit(200);

    dup2(fd,STDOUT_FILENO);

    dup2(fd,STDERR_FILENO);

    close(fd);

    execl("/bin/sh","/bin/sh","-c",cmd,(void *) 0);

    _exit(250);
  }

  return pid;
}

extern void eprintf(const char *fmt,...)
{
  ASSERT_ARGS(fmt == 0,);

  va_list args;
  int fd = -1;

  if(logfile == 0)
  {
    fd = open(LOGFILE,O_WRONLY|O_APPEND|O_CREAT,0644);

    if(fd == -1)
      return;

    logfile = fdopen(fd,"ab");

    if(logfile == 0)
    {
      close(fd);

      return;
    }

    setbuf(logfile,0);
  }

  va_start(args,fmt);

  vfprintf(logfile,fmt,args);

  va_end(args);
}

extern void snprintf_append(char *s,size_t size,const char *fmt,...)
{
  ASSERT_ARGS(s == 0 || size == 0 || fmt == 0,);

  size_t len = strlen(s);
  va_list args;

  va_start(args,fmt);

  vsnprintf(s+len,size-len,fmt,args);

  va_end(args);
}

extern void *list_append(void *list,size_t n)
{
  ASSERT_ARGS(n <= sizeof(struct list),0);

  struct list *a = list;
  struct list *b = malloc(n);

  if(a == 0)
  {
    b->prev = 0;

    b->next = 0;
  }
  else
  {
    a->next = b;

    b->prev = a;

    b->next = 0;
  }

  return b;
}

extern void *list_find_start(void *list)
{
  ASSERT_ARGS(list == 0,0);

  struct list *p = list;

  while(p->prev != 0)
    p = p->prev;

  return p;
}

extern void *list_find_end(void *list)
{
  ASSERT_ARGS(list == 0,0);

  struct list *p = list;

  while(p->next != 0)
    p = p->next;

  return p;
}

extern void list_free(void *list,void (*cb) (void *))
{
  if(list == 0 || cb == 0)
    return;

  struct list *a = list_find_start(list);
  struct list *b = 0;

  while(a != 0)
  {
    b = a->next;

    cb(a);

    a = b;
  }
}

extern void free_string(void *p)
{
  if(p == 0)
    return;

  struct string *string = p;

  free(string->data);

  free(string);
}

extern void free_partition(void *p)
{
  if(p == 0)
    return;

  struct partition *partition = p;

  free(partition->type_s);

  free(partition->uuid);

  free(partition->name);

  free(partition);
}

extern struct device *read_device_data(const char *path)
{
  ASSERT_ARGS(path == 0,0);

  int fd = -1;
  blkid_probe probe = 0;
  blkid_topology topology = 0;
  unsigned long long sector_size = 0;
  struct stat st;
  enum devicetype type = 0;
  struct device *device = 0;
  blkid_partlist partlist = 0;
  blkid_parttable parttable = 0;
  const char *label = 0;
  char *uuid = 0;
  int i = 0;
  int j = 0;
  blkid_partition partition = 0;
  struct partition *partitions = 0;
  int partnum = 0;
  const char *partname = 0;
  const char *partuuid = 0;
  blkid_loff_t partstart = 0;
  blkid_loff_t partsize = 0;
  int parttype_n = 0;
  const char *parttype_s = 0;
  unsigned long long partflags = 0;
  int primary = 0;
  int extended = 0;
  int logical = 0;

  memzero(&st,sizeof(struct stat));

  fd = open(path,O_RDONLY);

  if(fd == -1)
    goto bail;

  probe = blkid_new_probe();

  if(probe == 0)
    goto bail;

  if(blkid_probe_set_device(probe,fd,0,0) != 0)
    goto bail;

  topology = blkid_probe_get_topology(probe);

  if(topology == 0)
    goto bail;

  sector_size = blkid_topology_get_physical_sector_size(topology);

  if(sector_size == 0)
    goto bail;

  if(fstat(fd,&st) != 0)
    goto bail;

  if(is_ide_disk(&st))
    type = DEVICETYPE_IDE;
  else if(is_scsi_disk(&st))
    type = DEVICETYPE_SCSI;
  else if(is_virtio_disk(&st))
    type = DEVICETYPE_VIRTIO;
  else if(is_mdadm_device(&st))
    type = DEVICETYPE_MDADM;
  else
    goto bail;

  partlist = blkid_probe_get_partitions(probe);

  if(partlist != 0)
  {
    parttable = blkid_partlist_get_table(partlist);

    if(parttable != 0)
    {
      label = blkid_parttable_get_type(parttable);

      if(strcmp(label,"dos") == 0 || strcmp(label,"gpt") == 0)
      {
        if(strcmp(label,"gpt") == 0)
        {
          uuid = get_gpt_disk_uuid(path);

          if(uuid == 0)
            goto bail;
        }

        i = 0;

        j = blkid_partlist_numof_partitions(partlist);

        if(j != -1)
        {
          while(i < j && (partition = blkid_partlist_get_partition(partlist,i)) != 0)
          {
            partitions = list_append(partitions,sizeof(struct partition));

            partnum = blkid_partition_get_partno(partition);

            partname = blkid_partition_get_name(partition);

            partuuid = blkid_partition_get_uuid(partition);

            partstart = blkid_partition_get_start(partition);

            partsize = blkid_partition_get_size(partition);

            parttype_n = blkid_partition_get_type(partition);

            parttype_s = blkid_partition_get_type_string(partition);

            partflags = blkid_partition_get_flags(partition);

            primary = blkid_partition_is_primary(partition);

            extended = blkid_partition_is_extended(partition);

            logical = blkid_partition_is_logical(partition);

            partitions->num = partnum;

            partitions->name = (partname != 0) ? strdup(partname) : 0;

            partitions->uuid = (partuuid != 0) ? strdup(partuuid) : 0;

            partitions->start = partstart;

            partitions->end = partstart + partsize - 1;

            partitions->sectors = partsize;

            partitions->type_n = parttype_n;

            partitions->type_s = (parttype_s != 0) ? strdup(parttype_s) : 0;

            partitions->flags = partflags;

            partitions->primary = (primary != 0);

            partitions->extended = (extended != 0);

            partitions->logical = (logical != 0);

            ++i;
          }

          if(partitions != 0)
            partitions = list_find_start(partitions);
        }
      }
      else
        label = "unknown";
    }
  }

  device = malloc(sizeof(struct device));

  device->prev = 0;

  device->next = 0;

  device->path = strdup(path);

  device->sector_size = sector_size;

  device->type = type;

  device->label = (label != 0) ? strdup(label) : 0;

  device->uuid = uuid;

  device->partitions = partitions;

bail:

  if(fd != -1)
    close(fd);

  if(probe != 0)
    blkid_free_probe(probe);

  return device;
}

extern bool write_device_data(const struct device *device)
{
  ASSERT_ARGS(device == 0 || device->label == 0 || device->partitions == 0,false);

  char cmd[_POSIX_ARG_MAX] = {0};
  const struct partition *part = 0;
  pid_t pid = -1;
  int status = 0;
  int code = -1;

  if(strcmp(device->label,"dos") == 0)
  {
    snprintf(cmd,sizeof(cmd),"echo -n -e '");

    part = device->partitions;

    while(part != 0)
    {
      snprintf_append(cmd,sizeof(cmd),"%llu %llu 0x%hhx %c\\n",part->start,part->sectors,part->type_n,(part->flags == 0x80) ? '*' : '-');

      part = part->next;
    }

    snprintf_append(cmd,sizeof(cmd),"' | sfdisk -u S -L %s",device->path);
  }
  else if(strcmp(device->label,"gpt") == 0)
  {
    snprintf(cmd,sizeof(cmd),"sgdisk --clear --disk-guid='%s'",(device->uuid != 0) ? device->uuid : "R");

    part = device->partitions;

    while(part != 0)
    {
      snprintf_append(cmd,sizeof(cmd)," --new=%llu:%llu:%llu",part->num,part->start,part->end);

      snprintf_append(cmd,sizeof(cmd)," --change-name=%llu:'%s'",part->num,part->name);

      snprintf_append(cmd,sizeof(cmd)," --partition-guid=%llu:'%s'",part->num,(part->uuid != 0) ? part->uuid : "R");

      snprintf_append(cmd,sizeof(cmd)," --typecode=%llu:'%s'",part->num,part->type_s);

      snprintf_append(cmd,sizeof(cmd)," --attributes=%llu:set:0x%llx",part->num,part->flags);

      part = part->next;
    }

    snprintf_append(cmd,sizeof(cmd)," %s",device->path);
  }
  else
  {
    eprintf(INVALID_TABLE_TEXT,device->label,device->path);
    return false;
  }

  eprintf(EXECUTE_START_TEXT,cmd);

  pid = execute(cmd);

  if(pid == -1 || waitpid(pid,&status,0) == -1)
  {
    eprintf("%s: %s",__func__,strerror(errno));
    return false;
  }

  if(!WIFEXITED(status) || (code = WEXITSTATUS(status)) != 0)
  {
    eprintf(PROCESS_EXIT_ERROR_TEXT,pid,code);
    return false;
  }

  eprintf(EXECUTE_STOP_TEXT,cmd);

  return true;
}

extern void free_device(struct device *device)
{
  if(device == 0)
    return;

  list_free(device->partitions,free_partition);

  free(device->uuid);

  free(device->label);

  free(device->path);

  free(device);
}

#ifdef NEWT
extern bool get_text_size(const char *text,int *width,int *height)
{
  ASSERT_ARGS(text == 0 || width == 0 || height == 0,false);

  wchar_t wc = 0;
  size_t n = 0;
  size_t len = strlen(text);
  mbstate_t mbs;
  int cw = 0;
  int w = 0;
  int h = 0;
  int i = 0;

  memzero(&mbs,sizeof(mbstate_t));

  while(true)
  {
    n = mbrtowc(&wc,text,len,&mbs);

    if(n == (size_t) -1 || n == (size_t) -2)
      return false;

    if(n == 0)
      break;

    switch(wc)
    {
      case L'\n':
        if(cw > w)
          w = cw;
        cw = 0;
        ++h;
        break;

      case L'\t':
        cw += 8;
        break;

      default:
        i = wcwidth(wc);
        if(i > 0)
          cw += i;
        break;
    }

    text += n;

    len -= n;
  }

  if(w == 0 && cw > 0)
    w = cw;
  else if(w == 0)
    w = 1;

  if(h == 0)
    h = 1;

  *width = w;

  *height = h;

  return true;
}

extern bool get_button_size(const char *text,int *width,int *height)
{
  ASSERT_ARGS(text == 0 || width == 0 || height == 0,false);

  if(!get_text_size(text,width,height))
    return false;

  *width += 5;

  *height += 3;

  return true;
}
#endif

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

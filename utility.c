#include "fwsetup.h"

struct list
{
  struct list *prev;
  struct list *next;
};

static FILE *redirect_std_stream(FILE *oldfp,int oldfd)
{
  assert(oldfp != 0);
  assert(oldfd != -1);

  int newfd = fileno(oldfp);
  FILE *newfp = 0;

  fclose(oldfp);

  close(newfd);

  dup2(oldfd,newfd);

  newfp = fdopen(newfd,"wb");

  setbuf(newfp,0);

  return newfp;
}

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
  return (major(st->st_rdev) == VIRTBLK_MAJOR);
}

static inline bool is_mdadm_device(const struct stat *st)
{
  return (major(st->st_rdev) == MD_MAJOR);
}

extern void *malloc0(size_t n)
{
  assert(n > 0);

  void *p = malloc(n);

  memzero(p,n);

  return p;
}

extern void eprintf(const char *s,...)
{
  assert(s != 0);

  va_list args;
  static bool prepared = false;

  if(!prepared)
  {
    int fd = open(LOGFILE,O_CREAT|O_TRUNC|O_WRONLY,0644);

    if(fd == -1)
      return;

    stderr = redirect_std_stream(stderr,fd);

#ifndef NEWT
    stdout = redirect_std_stream(stdout,fd);
#endif

    prepared = true;
  }

  va_start(args,s);

  vfprintf(stderr,s,args);

  va_end(args);
}

extern void *list_append(void *list,size_t n)
{
  assert(n > sizeof(struct list));

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
  assert(list != 0);

  struct list *p = list;

  while(p->prev != 0)
    p = p->prev;

  return p;
}

extern void *list_find_end(void *list)
{
  assert(list != 0);

  struct list *p = list;

  while(p->next != 0)
    p = p->next;

  return p;
}

extern void list_free(void *list,void (*cb) (void *))
{
  assert(list != 0);
  assert(cb != 0);

  struct list *a = list_find_start(list);
  struct list *b = 0;

  while(a != 0)
  {
    b = a->next;

    cb(a);

    free(a);

    a = b;
  }
}

extern void string_free(void *string)
{
  assert(string != 0);

  struct string *p = string;

  free(p->data);
}

extern struct device *read_device_data(const char *path)
{
  assert(path != 0);

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

            if(strcmp(label,"dos") == 0)
              partitions->type_n = parttype_n;
            else if(strcmp(label,"gpt") == 0)
              partitions->type_s = strdup(parttype_s);

            partitions->flags = partflags;

            partitions->primary = (primary != 0);

            partitions->extended = (extended != 0);

            partitions->logical = (logical != 0);

            ++i;
          }

          if(partitions)
            partitions = list_find_start(partitions);
        }
      }
      else
        label = "unknown";
    }
  }

  device = malloc0(sizeof(struct device));

  device->path = strdup(path);

  device->sector_size = sector_size;

  device->type = type;

  device->label = (label != 0) ? strdup(label) : 0;

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
  assert(device != 0);
  assert(device->label != 0);

  char cmd[LINE_MAX] = {0};
  const struct partition *part = 0;
  size_t len = 0;
  int n = 0;

  if(strcmp(device->label,"dos") == 0)
  {
    part = device->partitions;

    n = snprintf(cmd+len,sizeof(cmd)-len,"echo -n -e '");

    if(n > 0)
      len += n;

    while(part != 0)
    {
      n = snprintf(cmd+len,sizeof(cmd)-len,"%llu %llu 0x%hhx %c\\n",part->start,part->sectors,part->type_n,(part->flags == 0x80) ? '*' : '-');

      if(n > 0)
        len += n;

      part = part->next;
    }

    n = snprintf(cmd+len,sizeof(cmd)-len,"' | sfdisk -u S -L %s",device->path);

    if(n > 0)
      len += n;

    return true;
  }

  return true;
}

extern void free_device(struct device *device)
{
  if(device == 0)
    return;

  free(device->label);

  free(device->path);

  free(device);
}

#ifdef NEWT
extern bool get_text_size(const char *text,int *width,int *height)
{
  assert(text != 0);
  assert(width != 0);
  assert(height != 0);

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
  assert(text != 0);
  assert(width != 0);
  assert(height != 0);

  if(!get_text_size(text,width,height))
    return false;

  *width += 5;

  *height += 3;

  return true;
}
#endif

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

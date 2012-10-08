#include <linux/major.h>
#include <blkid.h>
#include "local.h"

enum devicetype
{
  DEVICETYPE_FILE,
  DEVICETYPE_DISK,
  DEVICETYPE_UNKNOWN
};

struct device
{
  char *path;
  long long size;
  long long sectorsize;
  long long alignment;
  long long sectors;
  enum devicetype type;
};

enum disktype
{
  DISKTYPE_DOS,
  DISKTYPE_GPT
};

struct partition
{
  struct disk *disk;
  int number;
  long long start;
  long long size;
  long long end;
  unsigned char dostype;
  bool dosactive;
  char gptname[37];
  char gptuuid[37];
  char gpttype[37];
  unsigned long long gptflags;
};

struct disk
{
  struct device *device;
  enum disktype type;
  bool modified;
  unsigned int dosuuid;
  char gptuuid[37];
  struct partition table[128];
  int size;
};

static inline bool isdisk(const struct stat *st)
{
  switch(major(st->st_rdev))
  {
    // IDE disks
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

    // SCSI disks
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

    // virtio disks
    case 253:
      return true;
    
    default:
      return false;
  }
}

static bool getuuid(struct disk *disk)
{
  char command[_POSIX_ARG_MAX] = {0};
  FILE *pipe = 0;
  char line[LINE_MAX] = {0};
  bool outputsuccess = false;
  bool exitsuccess = false;
  char *p = 0;

  if(disk->type == DISKTYPE_DOS)
    snprintf(command,_POSIX_ARG_MAX,"export LC_ALL=C;yes | fdisk -l '%s' 2> /dev/null | sed -rn 's|^Disk identifier: 0x([0-9a-fA-F]+)$|\1|p'",disk->device->path);
  else if(disk->type == DISKTYPE_GPT)
    snprintf(command,_POSIX_ARG_MAX,"export LC_ALL=C;yes | gdisk -l '%s' 2> /dev/null | sed -rn 's|^Disk identifier \\(GUID\\): ([0-9a-zA-Z-]+)$|\1|p'",disk->device->path);
  else
    return false;

  if((pipe = popen(command,"r")) == 0)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  outputsuccess = (fgets(line,LINE_MAX,pipe) != 0);

  exitsuccess = (pclose(pipe) != -1);

  if((p = strchr(line,'\n')) != 0)
    *p = 0;

  if(outputsuccess && exitsuccess)
  {
    if(disk->type == DISKTYPE_DOS)
      disk->dosuuid = strtoul(line,0,16);
    else if(disk->type == DISKTYPE_GPT)
      snprintf(disk->gptuuid,37,"%s",line);
    return true;
  }
  else
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
}

static inline long long alignsector(const struct device *device,long long sector)
{
  long long alignment = device->alignment;

  if((sector % alignment) == 0)
    return sector;

  return sector + (alignment - (sector % alignment));
}

extern struct device *device_open(const char *path)
{
  int fd = -1;
  struct stat st = {0};
  long long size = 0;
  long long sectorsize = 0;
  long long alignment = 0;
  long long sectors = 0;
  enum devicetype type = DEVICETYPE_UNKNOWN;
  struct device *device = 0;
  
  if(path == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }
  
  if((fd = open(path,O_RDONLY)) == -1 || fstat(fd,&st) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }

  if(S_ISBLK(st.st_mode) && (ioctl(fd,BLKGETSIZE64,&size) == -1 || ioctl(fd,BLKSSZGET,&sectorsize) == -1))
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }
  else if(S_ISREG(st.st_mode))
  {
    size = st.st_size;
    sectorsize = 512;
  }
  else
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }
  
  alignment = MEBIBYTE / sectorsize;
  
  sectors = size / sectorsize;
  
  if(size <= 0 || sectorsize <= 0 || (MEBIBYTE % sectorsize) != 0 || (size % sectorsize) != 0)
  {
    errno = ERANGE;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }
  
  if(S_ISREG(st.st_mode))
    type = DEVICETYPE_FILE;
  else if(isdisk(&st))
    type = DEVICETYPE_DISK;
  else
    type = DEVICETYPE_UNKNOWN;
  
  device = malloc0(sizeof(struct device));
  
  device->path = strdup(path);
  
  device->size = size;
  
  device->sectorsize = sectorsize;
  
  device->alignment = alignment;
  
  device->sectors = sectors;
  
  device->type = type;
  
bail:

  if(fd != -1)
    close(fd);
  
  return device;
}

extern void device_close(struct device *device)
{
  if(device == 0)
    return;
  
  free(device->path);
  
  free(device);
}

extern struct disk *disk_open(struct device *device)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_partlist partlist = 0;
  blkid_parttable parttable = 0;
  const char *label = 0;
  struct disk disk = {0};
  int i = 0;
  int j = 0;
  blkid_partition partition = 0;
  struct disk *result = 0;

  if(device == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }

  if((fd = open(device->path,O_RDONLY)) == -1)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }

  if((probe = blkid_new_probe()) == 0)
  {
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }

  if(blkid_probe_set_device(probe,fd,0,0) == -1)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    goto bail;
  }

  if(
    (partlist = blkid_probe_get_partitions(probe))   == 0 ||
    (parttable = blkid_partlist_get_table(partlist)) == 0 ||
    (label = blkid_parttable_get_type(parttable))    == 0
  )
  {
    fprintf(logfile,"%s: %s\n",__func__,_("no partition table"));
    goto bail;
  }

  disk.device = device;

  if(strcmp(label,"dos") == 0)
    disk.type = DISKTYPE_DOS;
  else if(strcmp(label,"gpt") == 0)
    disk.type = DISKTYPE_GPT;
  else
  {
    fprintf(logfile,"%s: %s\n",__func__,_("unknown partition table"));
    goto bail;
  }

  if(!getuuid(&disk))
    goto bail;

  if((j = blkid_partlist_numof_partitions(partlist)) > 0)
  {
    if((disk.type == DISKTYPE_DOS && j > 60) || (disk.type == DISKTYPE_GPT && j > 128))
    {
      fprintf(logfile,"%s: %s\n",__func__,_("partition table too big"));
      goto bail;
    }
    
    for( ; i < j && (partition = blkid_partlist_get_partition(partlist,i)) != 0 ; ++i )
    {
      struct partition *part = &disk.table[i];
      
      part->number = blkid_partition_get_partno(partition);
      
      part->start = blkid_partition_get_start(partition);
      
      part->size = blkid_partition_get_size(partition);
      
      part->end = part->start + part->size - 1;
      
      if(disk.type == DISKTYPE_DOS)
      {
        part->dostype = blkid_partition_get_type(partition);
        
        part->dosactive = (blkid_partition_get_flags(partition) == 0x80);
        
        continue;
      }
      
      if(disk.type == DISKTYPE_GPT)
      {
        if(blkid_partition_get_name(partition) != 0)
          snprintf(part->gptname,37,"%s",blkid_partition_get_name(partition));

        snprintf(part->gptuuid,37,"%s",blkid_partition_get_uuid(partition));

        snprintf(part->gpttype,37,"%s",blkid_partition_get_type_string(partition));
        
        part->gptflags = blkid_partition_get_flags(partition);
        
        continue;
      }
    }
    
    disk.size = j;
  }

  result = memdup(&disk,sizeof(struct disk));

  for( i = 0 ; i < j ; ++i )
    result->table[i].disk = result;

bail:

  if(fd != -1)
    close(fd);
  
  if(probe != 0)
    blkid_free_probe(probe);
  
  return result;
}

extern void disk_new_table(struct disk *disk,const char *type)
{
  struct device *device = 0;
  enum disktype disktype = 0;

  if(disk == 0 || type == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return;
  }
  
  device = disk->device;

  if(strcmp(type,"dos") == 0)
    disktype = DISKTYPE_DOS;
  else if(strcmp(type,"gpt") == 0)
    disktype = DISKTYPE_GPT;
  else
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return;
  }

  memset(disk,0,sizeof(struct disk));
  
  disk->device = device;
  
  disk->type = disktype;

  disk->modified = true;
}

extern int disk_create_partition(struct disk *disk,long long size)
{
  struct partition part = {0};

  if(disk == 0 || disk->size < 0 || size <= 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return 0;
  }
  
  if(disk->size == 0)
  {
    part.number = 1;
    part.start = disk->device->alignment;
  }
  else
  {
    struct partition *last = &disk->table[disk->size-1];
    part.number = last->number + 1;
    part.start = last->end + 1;
  }

  part.size = size / disk->device->sectorsize;

  part.end = part.start + part.size - 1;

  part.start = alignsector(disk->device,part.start);

  part.end = alignsector(disk->device,part.end) - 1;
  
  part.size = (part.end - part.start) + 1;

  if(
    (disk->type == DISKTYPE_DOS && part.number > 60)  ||
    (disk->type == DISKTYPE_GPT && part.number > 128) ||
    part.size >= disk->device->sectors                
  )
  {
    errno = ERANGE;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return 0;
  }

  memcpy(&disk->table[disk->size++],&part,sizeof(struct partition));

  return part.number;
}

extern void disk_delete_partition(struct disk *disk)
{
  struct partition *last = 0;

  if(disk == 0 || disk->size <= 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return;
  }
  
  last = &disk->table[--disk->size];
  
  memset(last,0,sizeof(struct partition));

  disk->modified = true;
}

extern void disk_close(struct disk *disk)
{
  if(disk == 0)
    return;

  free(disk);
}

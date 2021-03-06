#include <linux/major.h>
#include <blkid.h>
#include <glob.h>
#include "local.h"

#define EMPTY_PARTITION &(struct partition) {0}
#define GPT_BOOT_FLAG (1ULL << 2ULL)

#define DOS_DATA     0x83
#define DOS_SWAP     0x82
#define DOS_RAID     0xFD
#define DOS_LVM      0x8E
#define DOS_EFI      0xEF
#define DOS_EXTENDED 0x05
#define GPT_DATA     "0FC63DAF-8483-4772-8E79-3D69D8477DE4"
#define GPT_SWAP     "0657FD6D-A4AB-43C4-84E5-0933C84B4F4F"
#define GPT_RAID     "A19D880F-05FC-4D3B-A006-743F0F84911E"
#define GPT_LVM      "E6D6D379-F507-44C2-A23C-238F2A3DF928"
#define GPT_EFI      "C12A7328-F81F-11D2-BA4B-00A0C93EC93B"
#define GPT_BIOS     "21686148-6449-6E6F-744E-656564454649"

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
  long long sectors;
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
    error(strerror(errno));
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
    error(strerror(errno));
    return false;
  }
}

static bool zapdisk(const char *path)
{
  char command[_POSIX_ARG_MAX] = {0};
  
  snprintf(command,_POSIX_ARG_MAX,"sgdisk --zap-all '%s'",path);
  
  return execute(command,"/",0);
}

static inline long long alignsector(const struct device *device,long long sector)
{
  long long alignment = device->alignment;

  if((sector % alignment) == 0)
    return sector;

  return sector + (alignment - (sector % alignment));
}

static inline void getsectors(struct disk *disk)
{
  long long sectorsize = disk->device->sectorsize;
  long long sectors = disk->device->sectors - 1;

  if(disk->type == DISKTYPE_GPT)
    sectors -= 1 + ((128 * 128) / sectorsize);

  disk->sectors = sectors;
}

static bool newpartition(struct disk *disk,long long size,struct partition *part)
{
  struct partition *last = 0;
  
  if(disk->size == 0)
  {
    part->number = 1;
    part->start = disk->device->alignment;
  }
  else
  {
    last = &disk->table[disk->size-1];
    part->number = last->number + 1;
    part->start = last->end + 1;
  }

  part->size = size / disk->device->sectorsize;

  part->end = part->start + part->size - 1;

  part->start = alignsector(disk->device,part->start);

  part->end = alignsector(disk->device,part->end) - 1;

  if(part->end > disk->sectors)
    part->end = disk->sectors;
  
  part->size = (part->end - part->start) + 1;

  if(
    part->size >= disk->sectors               ||
    (last != 0 && last->end >= disk->sectors)               
  )
  {
    errno = ERANGE;
    error(strerror(errno));
    return false;
  }  

  return true;
}

extern struct device **device_probe_all(bool disk)
{
  glob_t ge = {0};
  int flags = 0;
  struct device **devices = 0;
  size_t i = 0;
  size_t n = 0;
  struct device *device = 0;

  if(disk == false)
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }
  
  if(disk)
  {
    if(glob("/dev/[hsv]d[a-z]",flags,0,&ge) != 0)
    {
      globfree(&ge);
      error(strerror(errno));
      return 0;
    }
    
    flags |= GLOB_APPEND;
  }
  
  devices = malloc(ge.gl_pathc * sizeof(struct device *));
  
  for( ; i < ge.gl_pathc ; ++i )
  {
    device = device_open(ge.gl_pathv[i]);
    
    if(device == 0)
      continue;
    
    devices[n++] = device;
  }
  
  devices[n] = 0;
  
  devices = realloc(devices,(n + 1) * sizeof(struct device *));
  
  globfree(&ge);
  
  return devices;
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
    error(strerror(errno));
    goto bail;
  }
  
  if((fd = open(path,O_RDONLY)) == -1 || fstat(fd,&st) == -1)
  {
    error(strerror(errno));
    goto bail;
  }

  if(S_ISBLK(st.st_mode) && (ioctl(fd,BLKGETSIZE64,&size) == -1 || ioctl(fd,BLKSSZGET,&sectorsize) == -1))
  {
    error(strerror(errno));
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
    error(strerror(errno));
    goto bail;
  }
  
  alignment = MEBIBYTE / sectorsize;
  
  sectors = size / sectorsize;
  
  if(size <= 0 || sectorsize <= 0 || (MEBIBYTE % sectorsize) != 0 || (size % sectorsize) != 0)
  {
    errno = ERANGE;
    error(strerror(errno));
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

extern const char *device_get_path(struct device *device)
{
  if(device == 0)
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }

  return device->path;
}

extern long long device_get_size(struct device *device)
{
  if(device == 0)
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }
  
  return device->size;
}

extern const char *device_get_type(struct device *device)
{
  if(device == 0)
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }
  
  if(device->type == DEVICETYPE_FILE)
    return "file";
  else if(device->type == DEVICETYPE_DISK)
    return "disk";
  else if(device->type == DEVICETYPE_UNKNOWN)
    return "unknown";
  else
    return 0;
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
    error(strerror(errno));
    goto bail;
  }

  if((fd = open(device->path,O_RDONLY)) == -1)
  {
    error(strerror(errno));
    goto bail;
  }

  if((probe = blkid_new_probe()) == 0)
  {
    error(strerror(errno));
    goto bail;
  }

  if(blkid_probe_set_device(probe,fd,0,0) == -1)
  {
    errno = EINVAL;
    error(strerror(errno));
    goto bail;
  }

  if(
    (partlist = blkid_probe_get_partitions(probe))   == 0 ||
    (parttable = blkid_partlist_get_table(partlist)) == 0 ||
    (label = blkid_parttable_get_type(parttable))    == 0
  )
  {
    error("no partition table");
    goto bail;
  }

  disk.device = device;

  if(strcmp(label,"dos") == 0)
    disk.type = DISKTYPE_DOS;
  else if(strcmp(label,"gpt") == 0)
    disk.type = DISKTYPE_GPT;
  else
  {
    error("unknown partition table");
    goto bail;
  }

  getsectors(&disk);

  if(!getuuid(&disk))
    goto bail;

  if((j = blkid_partlist_numof_partitions(partlist)) > 0)
  {
    if((disk.type == DISKTYPE_DOS && j > 60) || (disk.type == DISKTYPE_GPT && j > 128))
    {
      error("partition table too big");
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

extern struct disk *disk_open_empty(struct device *device,const char *type)
{
  struct disk disk = {0};

  if(device == 0 || type == 0 || (strcmp(type,"dos") != 0 && strcmp(type,"gpt") != 0))
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }
  
  disk.device = device;
  
  disk_new_table(&disk,type);

  return memdup(&disk,sizeof(struct disk));
}

extern void disk_new_table(struct disk *disk,const char *type)
{
  struct device *device = 0;
  enum disktype disktype = 0;

  if(disk == 0 || type == 0)
  {
    errno = EINVAL;
    error(strerror(errno));
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
    error(strerror(errno));
    return;
  }

  memset(disk,0,sizeof(struct disk));
  
  disk->device = device;
  
  disk->type = disktype;

  getsectors(disk);

  disk->modified = true;
}

extern int disk_create_partition(struct disk *disk,long long size)
{
  struct partition part = {0};

  if(disk == 0 || disk->size < 0 || size <= 0)
  {
    errno = EINVAL;
    error(strerror(errno));
    return -1;
  }

  if(!newpartition(disk,size,&part))
    return -1;
    
  if(
    (disk->type == DISKTYPE_DOS && part.number > 4)  ||
    (disk->type == DISKTYPE_GPT && part.number > 128)            
  )
  {
    errno = ERANGE;
    error(strerror(errno));
    return -1;
  }  

  if(disk->type == DISKTYPE_DOS)
    part.dostype = DOS_DATA;
  else if(disk->type == DISKTYPE_GPT)
    snprintf(part.gpttype,37,"%s",GPT_DATA);

  memcpy(&disk->table[disk->size++],&part,sizeof(struct partition));

  disk->modified = true;

  return disk->size - 1;
}

extern int disk_create_extended_partition(struct disk *disk)
{
  int i = 0;
  struct partition part = {0};
  
  if(disk == 0 || disk->size < 0 || disk->type != DISKTYPE_DOS)
  {
    errno = EINVAL;
    error(strerror(errno));
    return -1;
  }
  
  for( ; i < disk->size ; ++i )
    if(disk->table[i].dostype == DOS_EXTENDED)
    {
      errno = EINVAL;
      error(strerror(errno));
      return -1;
    }
  
  if(!newpartition(disk,disk->sectors,&part))
    return -1;
  
  part.dostype = DOS_EXTENDED;
  
  memcpy(&disk->table[disk->size++],&part,sizeof(struct partition));

  disk->modified = true;  
  
  return disk->size - 1;
}

extern int disk_create_logical_partition(struct disk *disk,long long size)
{
  int i = 0;
  struct partition *ext = 0;
  struct partition *last = 0;
  struct partition part = {0};
  
  if(disk == 0 || disk->size < 0 || disk->type != DISKTYPE_DOS || size <= 0)
  {
    errno = EINVAL;
    error(strerror(errno));
    return -1;
  }
  
  for( ; i < disk->size ; ++i )
  {
    struct partition *part = &disk->table[i];
    
    if(part->dostype == DOS_EXTENDED && ext == 0)
      ext = part;
    
    last = part;
  }
  
  if(ext == 0 || ext->number > 4)
  {
    errno = EINVAL;
    error(strerror(errno));
    return -1;
  }
    
  part.number = (ext == last) ? 5 : last->number + 1;
  
  part.start = last->end + 1 + disk->device->alignment;
  
  part.size = size / disk->device->sectorsize;
  
  part.end = part.start + part.size - 1;
  
  part.start = alignsector(disk->device,part.start);
  
  part.end = alignsector(disk->device,part.end) - 1;
  
  if(part.end > disk->sectors)
    part.end = disk->sectors;
  
  part.size = (part.end - part.start) + 1;
  
  if(
    part.size >= disk->sectors ||
    last->end >= disk->sectors ||
    part.number > 60
  )
  {
    errno = ERANGE;
    error(strerror(errno));
    return -1;
  }
  
  part.dostype = DOS_DATA;
  
  memcpy(&disk->table[disk->size++],&part,sizeof(struct partition));

  disk->modified = true;
  
  return disk->size - 1;
}

extern void disk_delete_partition(struct disk *disk)
{
  struct partition *last = 0;

  if(disk == 0 || disk->size <= 0)
  {
    errno = EINVAL;
    error(strerror(errno));
    return;
  }
  
  last = &disk->table[--disk->size];
  
  memset(last,0,sizeof(struct partition));

  disk->modified = true;
}

extern void disk_partition_set_purpose(struct disk *disk,int n,const char *purpose)
{
  struct partition *part = 0;

  if(disk == 0 || n < 0 || n > disk->size || purpose == 0)
  {
    errno = EINVAL;
    error(strerror(errno));
    return;
  }

  part = &disk->table[n];
  
  if(disk->type == DISKTYPE_DOS)
  {
    if(strcmp(purpose,"data") == 0)
      part->dostype = DOS_DATA;
    else if(strcmp(purpose,"swap") == 0)
      part->dostype = DOS_SWAP;
    else if(strcmp(purpose,"raid") == 0)
      part->dostype = DOS_RAID;
    else if(strcmp(purpose,"lvm") == 0)
      part->dostype = DOS_LVM;
    else if(strcmp(purpose,"efi") == 0)
      part->dostype = DOS_EFI;
    else if(strcmp(purpose,"extended") == 0)
      part->dostype = DOS_EXTENDED;
  }
  else if(disk->type == DISKTYPE_GPT)
  {
    if(strcmp(purpose,"data") == 0)
      snprintf(part->gpttype,37,"%s",GPT_DATA);
    else if(strcmp(purpose,"swap") == 0)
      snprintf(part->gpttype,37,"%s",GPT_SWAP);
    else if(strcmp(purpose,"raid") == 0)
      snprintf(part->gpttype,37,"%s",GPT_RAID);
    else if(strcmp(purpose,"lvm") == 0)
      snprintf(part->gpttype,37,"%s",GPT_LVM);
    else if(strcmp(purpose,"efi") == 0)
      snprintf(part->gpttype,37,"%s",GPT_EFI);
    else if(strcmp(purpose,"bios") == 0)
      snprintf(part->gpttype,37,"%s",GPT_BIOS);
  }
  
  disk->modified = true;
}

extern void disk_partition_set_active(struct disk *disk,int n,bool active)
{
  struct partition *part = 0;

  if(disk == 0 || n < 0 || n > disk->size)
  {
    errno = EINVAL;
    error(strerror(errno));
    return;
  }

  part = &disk->table[n];
  
  if(disk->type == DISKTYPE_DOS)
    part->dosactive = active;
  else if(disk->type == DISKTYPE_GPT)
  {
    if(active)
      part->gptflags |= GPT_BOOT_FLAG;
    else if((part->gptflags & GPT_BOOT_FLAG) != 0)
      part->gptflags ^= GPT_BOOT_FLAG;
  }
  
  disk->modified = true;
}

extern int disk_partition_get_count(struct disk *disk)
{
  return disk->size;
}

extern const char *disk_partition_get_purpose(struct disk *disk,int n)
{
  struct partition *part = 0;
  const char *purpose = "unknown";

  if(disk == 0 || n < 0 || n > disk->size)
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }

  part = &disk->table[n];
  
  if(disk->type == DISKTYPE_DOS)
  {
    if(part->dostype == DOS_DATA)
      purpose = "data";
    else if(part->dostype == DOS_SWAP)
      purpose = "swap";
    else if(part->dostype == DOS_RAID)
      purpose = "raid";
    else if(part->dostype == DOS_LVM)
      purpose = "lvm";
    else if(part->dostype == DOS_EFI)
      purpose = "efi";
    else if(part->dostype == DOS_EXTENDED)
      purpose = "extended";
  }
  else if(disk->type == DISKTYPE_GPT)
  {
     if(strcmp(part->gpttype,GPT_DATA) == 0)
       purpose = "data";
     else if(strcmp(part->gpttype,GPT_SWAP) == 0)
       purpose = "swap";
     else if(strcmp(part->gpttype,GPT_RAID) == 0)
       purpose = "raid";
     else if(strcmp(part->gpttype,GPT_LVM) == 0)
       purpose = "lvm";
     else if(strcmp(part->gpttype,GPT_EFI) == 0)
       purpose = "efi";
     else if(strcmp(part->gpttype,GPT_BIOS) == 0)
       purpose = "bios";
  }
  
  return purpose;
}

extern bool disk_partition_get_active(struct disk *disk,int n)
{
  struct partition *part = 0;
  bool active = false;
  
  if(disk == 0 || n < 0 || n > disk->size)
  {
    errno = EINVAL;
    error(strerror(errno));
    return false;
  }
  
  part = &disk->table[n];
  
  if(disk->type == DISKTYPE_DOS)
    active = part->dosactive;
  else if(disk->type == DISKTYPE_GPT)
    active = (part->gptflags & GPT_BOOT_FLAG) != 0;
  
  return active;
}

extern int disk_partition_get_number(struct disk *disk,int n)
{
  struct partition *part = 0;
  
  if(disk == 0 || n < 0 || n > disk->size)
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }
  
  part = &disk->table[n];
  
  return part->number;
}

extern long long disk_partition_get_size(struct disk *disk,int n)
{
  struct partition *part = 0;
  
  if(disk == 0 || n < 0 || n > disk->size)
  {
    errno = EINVAL;
    error(strerror(errno));
    return 0;
  }
  
  part = &disk->table[n];
  
  return part->size * disk->device->sectorsize;
}

extern bool disk_flush(struct disk *disk)
{
  char command[_POSIX_ARG_MAX] = {0};
  int i = 0;
  int j = 0;
  struct partition *part = 0;
  struct partition *prev = 0;
  size_t n = 0;

  if(disk == 0 || (disk->type != DISKTYPE_DOS && disk->type != DISKTYPE_GPT))
  {
    errno = EINVAL;
    error(strerror(errno));
    return false;
  }

  if(!disk->modified)
    return true;

  if(disk->type == DISKTYPE_DOS)
  {
    snprintf(command,_POSIX_ARG_MAX,"set -e;echo -n -e '");
    
    n = strlen(command);
    
    for( ; i < disk->size ; ++i )
    {
      part = &disk->table[i];
      
      if(prev != 0)
        j = part->number - prev->number;
      
      for( ; j > 1 ; --j )
      {
        snprintf(command+n,_POSIX_ARG_MAX-n,"0 0 0x00 -\\n");
        
        n = strlen(command);
      }
      
      snprintf(command+n,_POSIX_ARG_MAX-n,"%lld %lld 0x%.2hhx %c\\n",
        part->start,
        part->size,
        part->dostype,
        (part->dosactive) ? '*' : '-'
      );
      
      n = strlen(command);
      
      prev = part;
    }
    
    snprintf(command+n,_POSIX_ARG_MAX-n,"' | sfdisk --unit S --Linux '%s';echo -n -e 'x\\ni\\n0x%.8x\\nw\\n' | fdisk '%s';",
      disk->device->path,
      (disk->dosuuid == 0) ? (unsigned int) rand() : disk->dosuuid,
      disk->device->path
    );
    
    n = strlen(command);    
  }
  else if(disk->type == DISKTYPE_GPT)
  {
    snprintf(command,_POSIX_ARG_MAX,"set -e;sgdisk --clear --disk-guid='%s'",
      (strlen(disk->gptuuid) == 0) ? "R" : disk->gptuuid
    );
    
    n = strlen(command);
    
    for( ; i < disk->size ; ++i )
    {
      part = &disk->table[i];
      
      snprintf(command+n,_POSIX_ARG_MAX-n," --new='%d:%lld:%lld' --change-name='%d:%s' --partition-guid='%d:%s' --typecode='%d:%s' --attributes='%d:=:0x%.16llx'",
        part->number,
        part->start,
        part->end,
        part->number,
        part->gptname,
        part->number,
        (strlen(part->gptuuid) == 0) ? "R" : part->gptuuid,
        part->number,
        part->gpttype,
        part->number,
        part->gptflags
      );

      n = strlen(command);
    }
    
    snprintf(command+n,_POSIX_ARG_MAX-n," '%s';",
      disk->device->path
    );
    
    n = strlen(command);  
  }
  
  if(!zapdisk(disk->device->path))
    return false;
  
  if(!execute(command,"/",0))
    return false;
  
  return true;
}

extern void disk_close(struct disk *disk)
{
  if(disk == 0)
    return;

  free(disk);
}

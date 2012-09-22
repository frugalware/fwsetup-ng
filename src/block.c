#include <linux/major.h>
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

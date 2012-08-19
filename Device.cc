#include <blkid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/major.h>
#include "Device.hh"

#define VIRTBLK_MAJOR 253

static inline bool isIdeDisk(const struct stat &st)
{
  switch(major(st.st_rdev))
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

static inline bool isScsiDisk(const struct stat &st)
{
  switch(major(st.st_rdev))
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

static inline bool isVirtioDisk(const struct stat &st)
{
  return (major(st.st_rdev) == VIRTBLK_MAJOR);
}

Device::Device()
{
  _lsectorsize = 0;

  _psectorsize = 0;

  _alignratio = 0;

  _sectors = 0;

  _disk = false;
}

Device::~Device()
{
}

bool Device::read(const string &path)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_topology topology = 0;
  unsigned long long lsectorsize = 0;
  unsigned long long psectorsize = 0;
  unsigned long long alignratio = 0;
  blkid_loff_t size = 0;
  unsigned long long sectors = 0;
  struct stat st;
  bool disk = false;
  bool rv = false;

  memset(&st,0,sizeof(struct stat));

  // Open file descriptor as read-only.
  if((fd = open(path.c_str(),O_RDONLY)) == -1)
    goto bail;

  // Create a new blkid probe.
  if((probe = blkid_new_probe()) == 0)
    goto bail;

  // Setup probe for the file descriptor we opened earlier.
  if(blkid_probe_set_device(probe,fd,0,0) == -1)
    goto bail;

  // Acquire the topology object.
  if((topology = blkid_probe_get_topology(probe)) == 0)
    goto bail;

  lsectorsize = blkid_topology_get_logical_sector_size(topology);

  psectorsize = blkid_topology_get_physical_sector_size(topology);

  alignratio = psectorsize / lsectorsize;

  size = blkid_probe_get_size(probe);

  sectors = size / lsectorsize;

  // Now, perform some sanity checks on the topology data.
  if(lsectorsize == 0 || psectorsize == 0 || (psectorsize % lsectorsize) != 0 || size <= 0 || (size % lsectorsize) != 0)
    goto bail;

  // Read the device's stats.
  if(fstat(fd,&st) == -1)
    goto bail;

  // Is it a physical disk?
  if(isIdeDisk(st) || isScsiDisk(st) || isVirtioDisk(st))
    disk = true;
  else
    goto bail;

  // Now, assign the details we've assembled.

  _path = path;

  _lsectorsize = lsectorsize;

  _psectorsize = psectorsize;

  _alignratio = alignratio;

  _sectors = sectors;

  _disk = disk;

bail:

  if(fd != -1)
    close(fd);

  if(probe != 0)
    blkid_free_probe(probe);

  return rv;
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

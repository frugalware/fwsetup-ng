#include <blkid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "Device.hh"

Device::Device()
{
  _path = "";

  _lsectorsize = 0;

  _psectorsize = 0;

  _alignratio = 0;

  _sectors = 0;
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
  bool rv = false;

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

  // Now, some sanity checks on the topology data.
  if(lsectorsize == 0 || psectorsize == 0 || (psectorsize % lsectorsize) != 0 || size <= 0 || (size % lsectorsize) != 0)
    goto bail;

  // Now, assign the details we've assembled.

  _path = path;

  _lsectorsize = lsectorsize;

  _psectorsize = psectorsize;

  _alignratio = alignratio;
  
  _sectors = sectors;

bail:

  if(fd != -1)
    close(fd);

  if(probe != 0)
    blkid_free_probe(probe);

  return rv;
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

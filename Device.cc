#include <blkid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/major.h>
#include "Utility.hh"
#include "DosPartitionTable.hh"
#include "GptPartitionTable.hh"
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
  _sectorsize = 0;

  _alignment = 0;

  _sectors = 0;

  _disk = false;

  _table = 0;

  _initialized = false;
}

Device::~Device()
{
  delete _table;
}

vector <Device> Device::probeAll()
{
  blkid_cache cache = 0;
  blkid_dev_iterate iter = 0;
  blkid_dev dev = 0;
  Device device;
  vector <Device> devices;

  if(blkid_get_cache(&cache,"/dev/null") != 0)
    goto bail;

  if(blkid_probe_all(cache) != 0)
    goto bail;

  if((iter = blkid_dev_iterate_begin(cache)) == 0)
    goto bail;

  while(blkid_dev_next(iter,&dev) == 0)
    if(device.read(blkid_dev_devname(dev)))
      devices.push_back(device);

bail:

  if(iter != 0)
    blkid_dev_iterate_end(iter);

  if(cache != 0)
    blkid_put_cache(cache);

  return devices;
}

bool Device::read(const string &path)
{
  int fd = -1;
  blkid_probe probe = 0;
  blkid_topology topology = 0;
  unsigned long long sectorsize = 0;
  unsigned long long alignment = 0;
  blkid_loff_t size = 0;
  unsigned long long sectors = 0;
  struct stat st;
  bool disk = false;
  PartitionTable *table = 0;
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

  sectorsize = blkid_topology_get_logical_sector_size(topology);

  alignment = MEBIBYTE / sectorsize;

  size = blkid_probe_get_size(probe);

  sectors = size / sectorsize;
  
  // Now, perform some sanity checks on the topology data.
  if(sectorsize == 0 || (MEBIBYTE % sectorsize) != 0 || size <= 0 || (size % sectorsize) != 0)
    goto bail;

  // Read the device's stats.
  if(fstat(fd,&st) == -1)
    goto bail;

  // Is it a physical disk?
  if(isIdeDisk(st) || isScsiDisk(st) || isVirtioDisk(st))
    disk = true;
//  else
//    goto bail;

  table = new DosPartitionTable();

  if(!table->read(path))
  {
    delete table;

    table = new GptPartitionTable();
    
    if(!table->read(path))
    {
      delete table;
      
      table = 0;
    }
  }

  // Now, assign the details we've assembled.

  _path = path;

  _sectorsize = sectorsize;

  _alignment = alignment;

  _sectors = sectors;

  _disk = disk;

  _table = table;

  _initialized = true;

  rv = true;

bail:

  if(fd != -1)
    close(fd);

  if(probe != 0)
    blkid_free_probe(probe);

  return rv;
}

bool Device::write()
{
  // TODO: block writes on non-physical devices
  if(!_initialized || _table == 0 || _table->getTableSize() == 0)
    return false;
  
  return _table->write(_path);
}

void Device::newPartitionTable(const string &label)
{
  if(!_initialized || (label != "dos" && label != "gpt"))
    return;
  
  delete _table;
  
  if(label == "dos")
    _table = new DosPartitionTable();
  else if(label == "gpt")
    _table = new DosPartitionTable();
}

Partition *Device::newPartition(unsigned long long size)
{
  unsigned long long sectors = 0;
  unsigned long long usable_sectors = 0;
  string label;
  size_t last = 0;
  Partition *part = 0;
  Partition *lastpart = 0;
  
  // Check for a sane state.
  if(!_initialized || size == 0 || _table == 0)
    return 0;

  sectors = sizeToSectors(size);

  usable_sectors = getUsableSectors();

  label = _table->getLabelType();
  
  last = _table->getTableSize();

  // Initial checks for resource limits.
  if(sectors > usable_sectors || (label == "dos" && (last+1) > 60) || (label == "gpt" && (last+1) > 128))
    return 0;

  part = _table->newPartition();

  if(last == 0)
  {
    part->setNumber(1);
  
    part->setStart(_alignment);    
    
    part->setEnd(_alignment + sectors);
  }
  else
  {
    lastpart = _table->getPartition(last - 1);
    
    part->setNumber(lastpart->getNumber() + 1);
    
    part->setStart(lastpart->getEnd() + 1);
    
    part->setEnd(lastpart->getEnd() + 1 + sectors);
  }

  part->setStart(alignUp(part->getStart()));

  part->setEnd(alignUp(part->getEnd()) - 1);

  if(part->getEnd() > usable_sectors)
    part->setEnd(usable_sectors - 1);

  part->setSectors(part->getEnd() - part->getStart() + 1);

  if(part->getStart() > usable_sectors)
  {
    delete part;

    return 0;
  }

  _table->putPartition(part);

  return part;
}

void Device::deleteLastPartition()
{
  if(!_initialized || _table == 0)
    return;

  _table->deleteLastPartition();  
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

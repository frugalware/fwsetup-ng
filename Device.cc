#include <blkid.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <glob.h>
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

vector <Device *> Device::probeAll()
{
  glob_t gl;
  Device *device = 0;
  vector <Device *> devices;

  memset(&gl,0,sizeof(glob_t));

  if(glob("/dev/[hsv]d[a-z]",0,0,&gl) != 0)
  {
    globfree(&gl);
    return devices;
  }

  for( size_t i = 0 ; i < gl.gl_pathc ; ++i )
  {
    const char *path = gl.gl_pathv[i];
    
    device = new Device();
    
    if(device->read(path))
      devices.push_back(device);
    else
      delete device;
  }

  globfree(&gl);

  return devices;
}

void Device::deleteAll(vector <Device *> &devices)
{
  while(!devices.empty())
  {
    delete devices.back();

    devices.pop_back();
  }
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
    _table = new GptPartitionTable();
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
  if(sectors > usable_sectors || (label == "dos" && (last+1) > 4) || (label == "gpt" && (last+1) > 128))
    return 0;

  if(last > 0 && label == "dos")
    for( size_t n = 0 ; n < last ; ++n )
      if(_table->getPartition(n)->getPurpose() == "extended")
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
    part->setEnd(usable_sectors);

  part->setSectors(part->getEnd() - part->getStart() + 1);

  if(part->getStart() > usable_sectors)
  {
    delete part;

    return 0;
  }

  _table->putPartition(part);

  return part;
}

Partition *Device::newExtendedPartition()
{
  Partition *part = 0;

  if(!_initialized || _table == 0 || _table->getLabelType() != "dos")
    return 0;

  if((part = newPartition(sectorsToSize(getUsableSectors()))) == 0)
    return 0;
  
  part->setPurpose("extended");
  
  return part;
}

Partition *Device::newLogicalPartition(unsigned long long size)
{
  Partition *extpart = 0;
  Partition *lastpart = 0;
  unsigned long long sectors = 0;
  unsigned long long usable_sectors = 0;
  Partition *logpart = 0;

  if(_table->getLabelType() != "dos")
    return 0;

  for( size_t n = 0 ; n < _table->getTableSize() ; ++n )
  {
    Partition *part = _table->getPartition(n);
    
    if(part->getPurpose() == "extended" && extpart == 0)
      extpart = part;

    lastpart = part;
  }

  if(extpart == 0 || extpart->getNumber() > 4)
    return 0;

  sectors = sizeToSectors(size);

  usable_sectors = getUsableSectors();

  if(sectors > usable_sectors || (lastpart->getNumber()+1) > 60)
    return 0;

  logpart = _table->newPartition();

  if(lastpart->getNumber() < 5)
  {
    logpart->setNumber(5);
    
    logpart->setStart(extpart->getStart() + _alignment);
    
    logpart->setEnd(extpart->getStart() + _alignment + sectors);
  }
  else
  {
    logpart->setNumber(lastpart->getNumber() + 1);
    
    logpart->setStart(lastpart->getEnd() + 1 + _alignment);
    
    logpart->setEnd(lastpart->getEnd() + 1 + _alignment + sectors);
  }

  logpart->setStart(alignUp(logpart->getStart()));

  logpart->setEnd(alignUp(logpart->getEnd()) - 1);

  if(logpart->getEnd() > usable_sectors)
    logpart->setEnd(usable_sectors - 1);

  logpart->setSectors(logpart->getEnd() - logpart->getStart() + 1);

  if(logpart->getStart() > usable_sectors)
  {
    delete logpart;
    return 0;
  }

  _table->putPartition(logpart);

  return logpart;
}

void Device::deleteLastPartition()
{
  if(!_initialized || _table == 0)
    return;

  _table->deleteLastPartition();  
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

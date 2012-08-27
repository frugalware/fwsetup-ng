#pragma once

#include <string>
#include <vector>
#include "PartitionTable.hh"

using std::string;
using std::vector;

class Device
{

public:
  Device();
  ~Device();
  static vector <Device> probeAll();
  bool read(const string &path);
  bool write();
  string getLabelType() { return (_table != 0) ? _table->getLabelType() : "unknown"; }
  size_t getTableSize() { return (_table != 0) ? _table->getTableSize() : 0; }
  Partition *getPartition(size_t n) { return (_table != 0) ? _table->getPartition(n) : 0; }
  void newPartitionTable(const string &label);
  Partition *newPartition(unsigned long long size);
  void deleteLastPartition();

private:
  unsigned long long sizeToSectors(unsigned long long size) { return size / _sectorsize; }
  unsigned long long sectorsToSize(unsigned long long sectors) { return sectors * _sectorsize; }
  unsigned long long alignUp(unsigned long long sector)
  {
    if((sector % _alignment) == 0)
      return sector;
    
    return sector + (_alignment - (sector % _alignment));
  }
  unsigned long long alignDown(unsigned long long sector)
  {
    if((sector % _alignment) == 0)
      return sector;
    
    return sector - (sector % _alignment);
  }
  unsigned long long getUsableSectors()
  {
    unsigned long long reserved = 0;

    // Set the raw size reserved for partition table.
    if(_table->getLabelType() == "dos")
      reserved = 512;
    else if(_table->getLabelType() == "gpt")
      reserved = 512 + 16384;
    
    // Now, round it up to the closest sector.
    if((reserved % _sectorsize) != 0)
      reserved += _sectorsize - (reserved % _sectorsize);
    
    // Now, convert it to a sector count.
    reserved = sizeToSectors(reserved);
    
    return _sectors - reserved;
  }
  string _path;
  unsigned long long _sectorsize;
  unsigned long long _alignment;
  unsigned long long _sectors;
  bool _disk;
  PartitionTable *_table;
  bool _initialized;

};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

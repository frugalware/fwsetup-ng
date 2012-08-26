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
  unsigned long long sizeToSectors(unsigned long long size) { return size / _sectorsize; }
  unsigned long long sectorsToSize(unsigned long long sectors) { return sectors * _sectorsize; }
  string getLabelType() { return (_table != 0) ? _table->getLabelType() : "unknown"; }
  void newPartitionTable(const string &label);

private:
  string _path;
  unsigned long long _sectorsize;
  unsigned long long _alignment;
  unsigned long long _sectors;
  bool _disk;
  PartitionTable *_table;
  bool _initialized;

};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

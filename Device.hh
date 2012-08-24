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
  unsigned long long sizeToSectors(unsigned long long size) { return size / _lsectorsize; }
  string getLabel() { (_table != 0) ? _table->getName() : "unknown"; }

private:
  string _path;
  unsigned long long _lsectorsize;
  unsigned long long _psectorsize;
  unsigned long long _alignratio;
  unsigned long long _sectors;
  bool _disk;
  PartitionTable *_table;

};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

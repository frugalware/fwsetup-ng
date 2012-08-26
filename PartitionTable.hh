#pragma once

#include <vector>
#include "Partition.hh"

using std::vector;

class PartitionTable
{

public:
  PartitionTable() { }
  virtual ~PartitionTable() { }
  virtual bool read(const string &path) { return true; }
  virtual bool write(const string &path) { return true; }
  string getLabelType() { return _label; }
  size_t getTableSize() { return _table.size(); }
  Partition *getPartition(size_t n) { return _table.at(n); }
  virtual Partition *newPartition() { return 0; }

protected:
  string _label;
  vector <Partition *> _table;

};

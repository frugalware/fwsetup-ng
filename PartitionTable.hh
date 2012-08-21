#pragma once

#include <vector>
#include "Partition.hh"

using std::vector;

class PartitionTable
{

public:
  PartitionTable() { }
  ~PartitionTable() { }
  virtual bool read(const string &path) { return true; }
  virtual bool write(const string &path) { return true; }

protected:
  string _label;
  vector <Partition *> _table;

};

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
  virtual string getName() { return ""; }

protected:
  string _label;
  vector <Partition *> _table;

};

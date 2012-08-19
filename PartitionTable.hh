#pragma once

#include <vector>
#include "Partition.hh"

using std::vector;

class PartitionTable
{

public:
  PartitionTable();
  ~PartitionTable();
  virtual bool read(const string &path);
  virtual bool write(const string &path);

protected:
  string _label;
  vector <Partition> _table;

};

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
  string label;
  vector <Partition> table;

};

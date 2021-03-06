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
  void putPartition(Partition *part) { _table.push_back(part); }
  virtual Partition *newPartition() { return 0; }
  void deleteLastPartition()
  {
    if(!_table.empty())
    {
      delete _table.back();
      _table.pop_back();
    }
  }

protected:
  string _label;
  vector <Partition *> _table;

};

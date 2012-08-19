#pragma once

#include "PartitionTable.hh"

class DosPartitionTable : public PartitionTable
{

public:
  virtual bool read(const string &path);
  virtual bool write(const string &path);  

};
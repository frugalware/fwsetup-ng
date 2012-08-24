#pragma once

#include "PartitionTable.hh"

class DosPartitionTable : public PartitionTable
{

public:
  DosPartitionTable();
  virtual ~DosPartitionTable();
  virtual bool read(const string &path);
  virtual bool write(const string &path);  
  virtual string getName() { return "dos"; }

};
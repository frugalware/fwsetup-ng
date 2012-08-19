#pragma once

#include "PartitionTable.hh"

class GptPartitionTable : public PartitionTable
{

public:
  virtual bool read(const string &path);
  virtual bool write(const string &path);  

protected:
  string _uuid;

};
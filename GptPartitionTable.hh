#pragma once

#include "PartitionTable.hh"

class GptPartitionTable : public PartitionTable
{

public:
  GptPartitionTable();
  virtual ~GptPartitionTable();
  virtual bool read(const string &path);
  virtual bool write(const string &path);  
  virtual string getName() { return "gpt"; }

protected:
  string _uuid;

};
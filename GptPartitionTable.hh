#pragma once

#include "PartitionTable.hh"

class GptPartitionTable : public PartitionTable
{

public:
  GptPartitionTable();
  virtual ~GptPartitionTable();
  virtual bool read(const string &path);
  virtual bool write(const string &path);  
  virtual Partition *newPartition();
  virtual unsigned long long getNumber(unsigned long long n);
  virtual unsigned long long getStart(unsigned long long n);
  virtual unsigned long long getEnd(unsigned long long n);
  virtual unsigned long long getSectors(unsigned long long n);
  virtual string getType(unsigned long long n);
  virtual bool getActive(unsigned long long n);

protected:
  string _uuid;

};
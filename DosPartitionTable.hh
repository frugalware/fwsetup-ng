#pragma once

#include "PartitionTable.hh"

class DosPartitionTable : public PartitionTable
{

public:
  DosPartitionTable();
  virtual ~DosPartitionTable();
  virtual bool read(const string &path);
  virtual bool write(const string &path);  
  virtual unsigned long long getNumber(unsigned long long n);
  virtual unsigned long long getStart(unsigned long long n);
  virtual unsigned long long getEnd(unsigned long long n);
  virtual unsigned long long getSectors(unsigned long long n);
  virtual string getType(unsigned long long n);
  virtual bool getActive(unsigned long long n);

};

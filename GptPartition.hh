#pragma once

#include "Partition.hh"

class GptPartition : public Partition
{

public:
  GptPartition() { _gpt_flags = 0; }
  virtual ~GptPartition() {}
  void setType(const string &type) { _gpt_type = type; }
  void setName(const string &name) { _gpt_name = name; }
  void setUUID(const string &uuid) { _gpt_uuid = uuid; }
  void setFlags(unsigned long long flags) { _gpt_flags = flags; }
  string getType() { return _gpt_type; }
  string getName() { return _gpt_name; }
  string getUUID() { return _gpt_uuid; }
  unsigned long long getFlags() { return _gpt_flags; } 

protected:
  string _gpt_type;
  string _gpt_name;
  string _gpt_uuid;
  unsigned long long _gpt_flags;

};
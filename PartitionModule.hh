#pragma once

#include "Module.hh"

class PartitionModule : public Module
{

public:
  PartitionModule();
  virtual ~PartitionModule();
  virtual int run();
  virtual string getName();

};

extern PartitionModule partition_module;
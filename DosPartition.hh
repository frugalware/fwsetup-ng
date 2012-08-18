#pragma once

#include "Partition.hh"

class DosPartition : Partition
{

public:
  DosPartition();
  ~DosPartition();
  virtual void setActive(bool active);

protected:
  unsigned char _dos_type;
  bool _dos_active;

};
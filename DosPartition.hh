#pragma once

#include "Partition.hh"

class DosPartition : Partition
{

public:
  virtual void setActive(bool active);

protected:
  unsigned char _dos_type;
  bool _dos_active;

};

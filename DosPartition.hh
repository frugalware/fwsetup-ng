#pragma once

#include "Partition.hh"

class DosPartition : public Partition
{

public:
  DosPartition();
  ~DosPartition();
  void setType(unsigned char type);
  virtual void setActive(bool active);

protected:
  unsigned char _dos_type;
  bool _dos_active;

};

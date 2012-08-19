#pragma once

#include "Partition.hh"

class DosPartition : public Partition
{

public:
  DosPartition() { _dos_type = 0, _dos_active = false; }
  ~DosPartition() { }
  void setType(unsigned char type) { _dos_type = type; }
  virtual void setActive(bool active) { _dos_active = true; }
  unsigned char getType() { return _dos_type; }
  virtual bool getActive(bool active) { return _dos_active; }

protected:
  unsigned char _dos_type;
  bool _dos_active;

};

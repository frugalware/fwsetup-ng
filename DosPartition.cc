#include "DosPartition.hh"

DosPartition::DosPartition()
{
  _dos_type = 0;
  
  _dos_active = false;
}

DosPartition::~DosPartition()
{
}

void DosPartition::setType(unsigned char type)
{
  _dos_type = type;
}

void DosPartition::setActive(bool active)
{
  _dos_active = active;
}

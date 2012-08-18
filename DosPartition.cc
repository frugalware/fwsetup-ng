#include "DosPartition.hh"

DosPartition::DosPartition()
{
  _dos_type = 0;
  
  _dos_active = false;
}

DosPartition::~DosPartition()
{
}

void DosPartition::setActive(bool active)
{
  _dos_active = active;
}

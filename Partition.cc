#include "Partition.hh"

Partition::Partition()
{
  _number = 0;

  _start = 0;

  _end = 0;

  _sectors = 0;
}

Partition::~Partition()
{
}

void Partition::setActive(bool active)
{
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

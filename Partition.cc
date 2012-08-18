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

void Partition::setNumber(unsigned long long number)
{
  if(_number != 0)
    return;

  _number = number;
}

void Partition::setStart(unsigned long long start)
{
  if(_start != 0)
    return;

  _start = start;
}

void Partition::setEnd(unsigned long long end)
{
  if(_end != 0)
    return;

  _end = end;
}

void Partition::setSectors(unsigned long long sectors)
{
  if(_sectors != 0)
    return;

  _sectors = sectors;
}

void Partition::setActive(bool active)
{
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

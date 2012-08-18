#include "Partition.hh"

Partition::Partition()
{
  _number = 0;

  _start = 0;

  _end = 0;

  _sectors = 0;

  _dos_type = 0;

  _dos_active = false;

  _gpt_name = "";

  _gpt_uuid = "";

  _gpt_type = "";

  _gpt_flags = 0;
}

Partition::~Partition()
{
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

#pragma once

#include <string>

using std::string;

class Partition
{

public:
  Partition();
  ~Partition();

private:
  unsigned long long _number;
  unsigned long long _start;
  unsigned long long _end;
  unsigned long long _sectors;
  unsigned char _dos_type;
  bool _dos_active;
  string _gpt_name;
  string _gpt_uuid;
  string _gpt_type;
  unsigned long long _gpt_flags;

};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

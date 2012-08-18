#pragma once

#include <string>

using std::string;

class Partition
{

public:
  Partition();
  ~Partition();
  virtual void setActive(bool active);

protected:
  unsigned long long _number;
  unsigned long long _start;
  unsigned long long _end;
  unsigned long long _sectors;

};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

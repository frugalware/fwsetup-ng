#pragma once

#include <string>

using std::string;

class Partition
{

public:
  Partition() { _number = 0, _start = 0, _end = 0, _sectors = 0; }
  virtual ~Partition() { }
  void setNumber(unsigned long long number) { _number = number; }
  void setStart(unsigned long long start) { _start = start; }
  void setEnd(unsigned long long end) { _end = end; }
  void setSectors(unsigned long long sectors) { _sectors = sectors; }
  virtual void setActive(bool active) { }
  virtual void setPurpose(string purpose) { }
  unsigned long long getNumber() { return _number; }
  unsigned long long getStart() { return _start; }
  unsigned long long getEnd() { return _end; }
  unsigned long long getSectors() { return _sectors; }
  virtual bool getActive() { return true; }
  virtual string getPurpose() { return ""; }

protected:
  unsigned long long _number;
  unsigned long long _start;
  unsigned long long _end;
  unsigned long long _sectors;

};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

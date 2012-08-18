#pragma once

#include <string>

using std::string;

class Device
{

public:
  Device();
  ~Device();
  bool read(const string &path);

private:
  string _path;
  unsigned long long _lsectorsize;
  unsigned long long _psectorsize;
  unsigned long long _alignratio;
  unsigned long long _sectors;
  bool _disk;

};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

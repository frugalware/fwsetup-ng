#pragma once

#include <string>

using std::string;

class Module
{

public:
  Module() {}
  virtual ~Module() {}
  virtual int run() { return 0; }
  virtual string getName() { return ""; }

};

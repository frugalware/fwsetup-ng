#pragma once

class UserInterface
{

public:
  UserInterface();
  ~UserInterface();
  bool initialize(int argc,char **argv);

private:
  int _s_width;
  int _s_height;
  int _w_width;
  int _w_height;
  int _x;
  int _y;
  bool _initialized;

};
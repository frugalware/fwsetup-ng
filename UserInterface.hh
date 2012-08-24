#pragma once

class UserInterface
{

public:
  UserInterface();
  ~UserInterface();
  bool initialize(int argc,char **argv);
  int getScreenWidth() { return _s_width; }
  int getScreenHeight() { return _s_height; }
  int getWindowWidth() { return _w_width; }
  int getWindowHeight() { return _w_height; }
  int getWindowXOffset() { return _x; }
  int getWindowYOffset() { return _y; }

private:
  int _s_width;
  int _s_height;
  int _w_width;
  int _w_height;
  int _x;
  int _y;
  bool _initialized;

};

extern UserInterface ui;

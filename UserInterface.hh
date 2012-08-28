#pragma once

#include <string>

using std::string;

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
  void textDialog(const string &title,const string &text);
#ifdef NEWT
  bool getTextSize(const string &text,int &width,int &height);
  bool getButtonSize(const string &text,int &width,int &height);
#endif

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

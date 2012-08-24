#include "UserInterface.hh"

#ifdef NEWT
#include <newt.h>
#endif

UserInterface::UserInterface()
{
  _s_width = 0;
  
  _s_height = 0;
  
  _w_width = 0;
  
  _w_height = 0;

  _x = 0;
  
  _y = 0;

  _initialized = false;
}

UserInterface::~UserInterface()
{
  if(_initialized)
  {
#ifdef NEWT
    newtFinished();
#endif
  }
}

bool UserInterface::initialize(int argc,char **argv)
{
#ifdef NEWT
  if(newtInit() != 0)
    return false;

  newtGetScreenSize(&_s_width,&_s_height);

  _w_width = (_s_width - 2) * 9 / 10;
  
  _w_height = (_s_height - 2) * 9 / 10;
  
  _x = (_s_width - _w_width) / 2;
  
  _y = (_s_height - _w_height) / 2;
#endif

  _initialized = true;

  return true;
}

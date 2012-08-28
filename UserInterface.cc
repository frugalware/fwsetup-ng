#include "UserInterface.hh"

#ifdef NEWT
#include <string.h>
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

#ifdef NEWT
bool UserInterface::getTextSize(const string &text,int &width,int &height)
{
  wchar_t wc = 0;
  size_t n = 0;
  size_t off = 0;
  mbstate_t mbs;
  int cw = 0;
  int w = 0;
  int h = 0;
  int i = 0;

  memset(&mbs,0,sizeof(mbstate_t));

  while(true)
  {
    n = mbrtowc(&wc,text.c_str()+off,text.length()-off,&mbs);

    if(n == (size_t) -1 || n == (size_t) -2)
      return false;

    if(n == 0)
      break;

    switch(wc)
    {
      case L'\t':
        cw += 8;
        break;

      case L'\n':
        if(cw > w)
          w = cw;
        cw = 0;
        ++h;
        break;

      default:
        if((i = wcwidth(wc)) > 0)
          cw += i;
        break;
    }

    off += n;
  }

  if(w == 0)
    w = (cw > 0) ? cw : 1;

  if(h == 0)
    h = 1;

  width = w;

  height = h;

  return true;
}

bool UserInterface::getButtonSize(const string &text,int &width,int &height)
{
  if(!getTextSize(text,width,height))
    return false;

  width += 5;

  height += 3;

  return true;
}
#endif

UserInterface ui;

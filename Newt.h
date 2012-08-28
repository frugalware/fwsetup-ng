#pragma once

#ifdef NEWT
extern "C"
{
#include <newt.h>

enum
{
  NEWT_EXIT_HOTKEY,
  NEWT_EXIT_COMPONENT,
  NEWT_EXIT_FDREADY,
  NEWT_EXIT_TIMER,
  NEWT_EXIT_ERROR
};
}
#endif

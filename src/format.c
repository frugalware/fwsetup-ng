#include "local.h"

static bool format_run(void)
{
  return true;
}

static void format_reset(void)
{
}

struct module format_module =
{
  format_run,
  format_reset,
  __FILE__
};

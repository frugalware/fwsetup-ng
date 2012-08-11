#include "fwsetup.h"

static enum order partition_setup_run(struct database *db)
{
	return 0;
}

struct module partition_setup_module =
{
  __FILE__,
  partition_setup_run
};

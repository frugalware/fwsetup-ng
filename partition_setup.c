#include "fwsetup.h"

static enum order partition_setup_run(struct database *db)
{
  assert(db != 0);

  struct device *device = read_device_data("test.img");

  free_device(device);

  return ORDER_NEXT;
}

struct module partition_setup_module =
{
  __FILE__,
  partition_setup_run
};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

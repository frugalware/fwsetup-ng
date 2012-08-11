#include "fwsetup.h"

#define MODULE_COUNT (2 + 2)
#define EMPTY_MODULE (&(struct module) { 0, 0 })

static struct database db;

static struct module modules[MODULE_COUNT];

extern int main(void)
{
  struct module *module = &modules[1];

  memzero(&db,sizeof(struct database));

  memzero(&modules,sizeof(struct module) * MODULE_COUNT);

  modules[1] = begin_module;

  modules[2] = end_module;

  db.locale = setlocale(LC_ALL,"");

#ifdef NEWT
  if(newtInit() != 0)
  {
    eprintf(NEWTINIT_TEXT);

    return EXIT_FAILURE;
  }

  newtCls();

  newtGetScreenSize(&db.screen_width,&db.screen_height);

  db.window_width = (int) ((double) (db.screen_width - 2) * 0.9);

  db.window_height = (int) ((double) (db.screen_height - 2) * 0.9);

  db.window_x = (db.screen_width - db.window_width) / 2;

  db.window_y = (db.screen_height - db.window_height) / 2;

  while(memcmp(module,EMPTY_MODULE,sizeof(struct module)) != 0)
    switch(module->run(&db))
    {
      case ORDER_NONE:
        assert_not_reached();
        break;

      case ORDER_ERROR:
        break;

      case ORDER_PREVIOUS:
        --module;
        break;

      case ORDER_NEXT:
        ++module;
        break;

      default:
        assert_not_reached();
        break;
    }

  newtFinished();
#endif

  return EXIT_SUCCESS;
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

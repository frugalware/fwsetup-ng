#include "fwsetup.h"

extern int main(void)
{
  struct database db;

  memzero(&db,sizeof(struct database));

#ifdef NEWT
  if(newtInit() != 0)
  {
    eprintf("Failed to initialize NEWT.\n");

    return EXIT_FAILURE;
  }

  newtCls();

  newtGetScreenSize(&db.screen_width,&db.screen_height);

  db.window_width = (int) ((double) (db.screen_width - 2) * 0.9);

  db.window_height = (int) ((double) (db.screen_height - 2) * 0.9);

  db.window_x = (db.screen_width - db.window_width) / 2;

  db.window_y = (db.screen_height - db.window_height) / 2;

  newtFinished();
#endif

  return EXIT_SUCCESS;
}

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

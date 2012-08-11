#include "fwsetup.h"

#define TEXTBOX_TEXT _(                        \
"Welcome to the Frugalware Linux Installer!\n" \
"\n"                                           \
"Please click 'Next' to continue.\n"           \
)

static enum order begin_run(struct database *db)
{
  assert(db != 0);

#ifdef NEWT
  int tb_width = 0;
  int tb_height = 0;
  int nb_width = 0;
  int nb_height = 0;
  newtComponent tb = 0;
  newtComponent nb = 0;
  newtComponent form = 0;
  struct newtExitStruct es;

  memzero(&es,sizeof(struct newtExitStruct));

  if(!get_text_size(TEXTBOX_TEXT,&tb_width,&tb_height) || !get_button_size(NEXTBUTTON_TEXT,&nb_width,&nb_height))
  {
    eprintf(TEXTSIZE_TEXT);
    return ORDER_ERROR;
  }

  if(newtOpenWindow(db->window_x,db->window_y,db->window_width,db->window_height,WINDOWTITLE_TEXT) != 0)
  {
    eprintf(NEWTWINDOW_TEXT);
    return ORDER_ERROR;
  }

  tb = newtTextbox(0,0,tb_width,tb_height,0);

  newtTextboxSetText(tb,TEXTBOX_TEXT);

  nb = newtButton(db->window_width-nb_width,db->window_height-nb_height,NEXTBUTTON_TEXT);

  form = newtForm(0,0,0);

  newtFormAddComponents(form,tb,nb,(void *) 0);

  newtFormSetCurrent(form,nb);

  while(true)
  {
    newtFormRun(form,&es);

    if(es.reason == NEWT_EXIT_COMPONENT && es.u.co == nb)
      break;

    memzero(&es,sizeof(struct newtExitStruct));
  }

  newtFormDestroy(form);

  newtPopWindow();
#endif

  return ORDER_NEXT;
}

struct module begin_module =
{
  __FILE__,
  begin_run
};

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

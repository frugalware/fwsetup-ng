#include <newt.h>
#include "local.h"

extern int ui_main(int argc,char **argv)
{
  int w = 0;
  int h = 0;

  if(newtInit() != 0)
  {
    fprintf(logfile,_("Could not initialize the NEWT user interface.\n"));
    return 1;
  }

  newtGetScreenSize(&w,&h);

  if(w < 80 || h < 24)
  {
    fprintf(logfile,_("We require a terminal of 80x24 or greater to use the NEWT user interface.\n"));
    newtFinished();
    return 1;
  }  

  newtCls();

  newtFinished();

  return 0;
}

extern void ui_dialog_text(const char *title,const char *text)
{
  int textbox_width = 0;
  int textbox_height = 0;
  int button_width = 0;
  int button_height = 0;
  newtComponent textbox = 0;
  newtComponent button = 0;
  newtComponent form = 0;
  struct newtExitStruct es = {0};

  if(title == 0 || text == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return;
  }

  if(!get_text_screen_size(text,&textbox_width,&textbox_height))
    return;

  if(!get_button_screen_size(OK_BUTTON_TEXT,&button_width,&button_height))
    return;

  if(newtCenteredWindow(NEWT_WIDTH,NEWT_HEIGHT,title) != 0)
  {
    fprintf(logfile,_("Failed to open a NEWT window.\n"));
    return;
  }
  
  textbox = newtTextbox(0,0,textbox_width,textbox_height,0);
  
  newtTextboxSetText(textbox,text);
  
  button = newtButton(NEWT_WIDTH-button_width,NEWT_HEIGHT-button_height,OK_BUTTON_TEXT);
  
  form = newtForm(0,0,NEWT_FLAG_NOF12);
  
  newtFormAddComponents(form,textbox,button,(void *) 0);
  
  newtFormSetCurrent(form,button);

  while(true)
  {  
    newtFormRun(form,&es);
    
    if(es.reason == NEWT_EXIT_COMPONENT && es.u.co == button)
      break;
  }
  
  newtFormDestroy(form);
  
  newtPopWindow();
}

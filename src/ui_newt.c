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

extern bool ui_dialog_yesno(const char *title,const char *text,bool defaultno)
{
  int textbox_width = 0;
  int textbox_height = 0;
  int yes_width = 0;
  int yes_height = 0;
  int no_width = 0;
  int no_height = 0;
  newtComponent textbox = 0;
  newtComponent yes = 0;
  newtComponent no = 0;
  newtComponent form = 0;
  struct newtExitStruct es = {0};
  bool result = false;

  if(title == 0 || text == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }
  
  if(!get_text_screen_size(text,&textbox_width,&textbox_height))
    return false;

  if(!get_button_screen_size(YES_BUTTON_TEXT,&yes_width,&yes_height))
    return false;
  
  if(!get_button_screen_size(NO_BUTTON_TEXT,&no_width,&no_height))
    return false;

  if(newtCenteredWindow(NEWT_WIDTH,NEWT_HEIGHT,title) != 0)
  {
    fprintf(logfile,_("Failed to open a NEWT window.\n"));
    return false;
  }

  textbox = newtTextbox(0,0,textbox_width,textbox_height,0);
  
  newtTextboxSetText(textbox,text);

  yes = newtButton(NEWT_WIDTH-yes_width-no_width,NEWT_HEIGHT-yes_height,YES_BUTTON_TEXT);
  
  no = newtButton(NEWT_WIDTH-no_width,NEWT_HEIGHT-no_height,NO_BUTTON_TEXT);
  
  form = newtForm(0,0,NEWT_FLAG_NOF12);
  
  newtFormAddComponents(form,textbox,yes,no,(void *) 0);
  
  newtFormSetCurrent(form,(defaultno) ? no : yes);
  
  while(true)
  {
    newtFormRun(form,&es);
    
    if(es.reason == NEWT_EXIT_COMPONENT && (es.u.co == yes || es.u.co == no))
    {
      result = (es.u.co == yes);
      break;
    }
  }
  
  newtFormDestroy(form);
  
  newtPopWindow();
  
  return result;
}

extern bool ui_dialog_progress(const char *title,ui_dialog_progress_callback cb,void *data)
{
  char text[NEWT_WIDTH + 1] = {0};
  int percent = 0;
  newtComponent label = 0;
  newtComponent scale = 0;
  newtComponent form = 0;
  struct newtExitStruct es = {0};
  bool result = false;

  if(title == 0 || cb == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(!cb(text,NEWT_WIDTH + 1,&percent,data))
  {
    fprintf(logfile,_("Progress dialog callback canceled the operation.\n"));
    return false;
  }

  if(newtCenteredWindow(NEWT_WIDTH,3,title) != 0)
  {
    fprintf(logfile,_("Failed to open a NEWT window.\n"));
    return false;
  }
  
  label = newtLabel(0,0,text);
  
  scale = newtScale(0,2,NEWT_WIDTH,100);
  
  newtScaleSet(scale,percent);
  
  form = newtForm(0,0,NEWT_FLAG_NOF12);
  
  newtFormAddComponents(form,label,scale,(void *) 0);
  
  newtFormSetTimer(form,100);
  
  while(true)
  {
    newtFormRun(form,&es);

    if(es.reason == NEWT_EXIT_TIMER)
    {
      if(!cb(text,NEWT_WIDTH + 1,&percent,data))
      {
        fprintf(logfile,_("Progress dialog callback canceled the operation.\n"));
        result = false;
        break;
      }

      newtLabelSetText(label,text);
      
      newtScaleSet(scale,percent);
    }
    
    if(percent > 100)
    {
      result = true;
      break;
    }
  }
  
  newtFormDestroy(form);
  
  newtPopWindow();
  
  return result;
}

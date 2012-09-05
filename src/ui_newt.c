#include <newt.h>
#include "local.h"

#define NEWT_WIDTH 70
#define NEWT_HEIGHT 21

extern int ui_main(int argc,char **argv)
{
  int w = 0;
  int h = 0;

  if(newtInit() != 0)
  {
    fprintf(logfile,NEWT_INIT_FAILURE_TEXT);
    return 1;
  }

  newtGetScreenSize(&w,&h);

  if(w < 80 || h < 24)
  {
    fprintf(logfile,NEWT_TOO_SMALL_TEXT);
    newtFinished();
    return 1;
  }  

  newtCls();

  install_module.run();

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
    fprintf(logfile,NEWT_WINDOW_FAILURE_TEXT);
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
    fprintf(logfile,NEWT_WINDOW_FAILURE_TEXT);
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

extern bool ui_dialog_progress(const char *title,const char *text,int percent)
{
  static char *oldtitle = 0;
  static newtComponent label = 0;
  static newtComponent scale = 0;
  static newtComponent form = 0;

  if((title == 0 && text == 0 && percent == -1) || (oldtitle != 0 && strcmp(oldtitle,title) != 0))
  {
    if(label != 0 && scale != 0 && form != 0 && oldtitle != 0)
    {
      free(oldtitle);
      newtFormDestroy(form);
      newtPopWindow();
      oldtitle = 0;
      label = 0;
      scale = 0;
      form = 0;
    }
    
    if(title == 0 && text == 0 && percent == -1)
      return true;
  }

  if(title == 0 || text == 0 || percent < 0 || percent > 100)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(label == 0 && scale == 0 && form == 0)
  {
    if(newtCenteredWindow(NEWT_WIDTH,3,title) != 0)
    {
      fprintf(logfile,NEWT_WINDOW_FAILURE_TEXT);
      return false;
    }
    
    oldtitle = strdup(title);
    
    label = newtLabel(0,0,"");
    
    scale = newtScale(0,2,NEWT_WIDTH,100);
    
    form = newtForm(0,0,NEWT_FLAG_NOF12);

    newtFormAddComponents(form,label,scale,(void *) 0);
  }

  newtLabelSetText(label,text);
  
  newtScaleSet(scale,percent);

  newtDrawForm(form);
  
  newtRefresh();

  return true;
}

extern int ui_window_install(const char *title,struct install *data)
{
  int textbox_width = 0;
  int textbox_height = 0;
  int previous_width = 0;
  int previous_height = 0;
  int next_width = 0;
  int next_height = 0;
  int checkboxtree_width = 0;
  int checkboxtree_height = 0;
  newtComponent textbox = 0;
  newtComponent previous = 0;
  newtComponent next = 0;
  newtComponent checkboxtree = 0;
  int i = 0;
  struct install *pkg = 0;
  newtComponent form = 0;
  struct newtExitStruct es = {0};
  int result = 0;

  if(!get_text_screen_size(INSTALL_WINDOW_TEXT,&textbox_width,&textbox_height))
    return 0;

  if(!get_button_screen_size(PREVIOUS_BUTTON_TEXT,&previous_width,&previous_height))
    return 0;
  
  if(!get_button_screen_size(NEXT_BUTTON_TEXT,&next_width,&next_height))
    return 0;

  checkboxtree_width = NEWT_WIDTH;
  
  checkboxtree_height = NEWT_HEIGHT - textbox_height - max(previous_height,next_height) - 2;

  if(newtCenteredWindow(NEWT_WIDTH,NEWT_HEIGHT,title) != 0)
  {
    fprintf(logfile,NEWT_WINDOW_FAILURE_TEXT);
    return 0;
  }
  
  textbox = newtTextbox(0,0,textbox_width,textbox_height,0);

  newtTextboxSetText(textbox,INSTALL_WINDOW_TEXT);

  previous = newtButton(NEWT_WIDTH-previous_width-next_width,NEWT_HEIGHT-previous_height,PREVIOUS_BUTTON_TEXT);

  next = newtButton(NEWT_WIDTH-next_width,NEWT_HEIGHT-next_height,NEXT_BUTTON_TEXT);

  checkboxtree = newtCheckboxTree(0,textbox_height+1,checkboxtree_height,NEWT_FLAG_SCROLL);

  newtCheckboxTreeSetWidth(checkboxtree,checkboxtree_width);

  pkg = data;

  while(pkg->name != 0)
  {
    newtCheckboxTreeAddItem(checkboxtree,pkg->name,&pkg->checked,0,i,NEWT_ARG_LAST);
    newtCheckboxTreeSetEntryValue(checkboxtree,&pkg->checked,(pkg->checked) ? '*' : ' ');
    ++i;
    ++pkg;
  }

  form = newtForm(0,0,NEWT_FLAG_NOF12);

  newtFormAddComponents(form,textbox,previous,next,checkboxtree,(void *) 0);

  newtFormSetCurrent(form,checkboxtree);

  while(true)
  {
    newtFormRun(form,&es);
    
    if(es.reason == NEWT_EXIT_COMPONENT && (es.u.co == previous || es.u.co == next))
    {
      result = (es.u.co == previous) ? -1 : 1;
      break;
    }
  }

  pkg = data;

  while(pkg->name != 0)
  {
    pkg->checked = (newtCheckboxTreeGetEntryValue(checkboxtree,&pkg->checked) == '*');
    ++pkg;
  }

  newtFormDestroy(form);

  newtPopWindow();

  return result;
}

#include <newt.h>
#include "local.h"

#define NEWT_WIDTH 70
#define NEWT_HEIGHT 21

extern int ui_main(int argc,char **argv)
{
  int w = 0;
  int h = 0;
  struct module *module = 0;
  size_t n = 0;
  char text[4096] = {0};
  int code = EXIT_SUCCESS;

  // This parameter is never used.
  argc = argc;

  // This parameter is never used.
  argv = argv;

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

  while(true)
  {
    module = modules[n];
  
    if(module == 0)
      break;
  
    if(module->run == 0 || module->reset == 0 || module->name == 0)
    {
      errno = EINVAL;
      fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
      code = EXIT_FAILURE;
      break;
    }
  
    fprintf(logfile,_("About to run module '%s'.\n"),module->name);
  
    bool success = module->run();
    
    if(!success)
    {
      fprintf(logfile,_("A fatal error has been reported by module '%s'.\n"),module->name);
      module->reset();
      snprintf(text,4096,_("A fatal error has been reported by module '%s'.\n\nFor more information, please see the logfile at '%s'. Thank you.\n"),module->name,LOGFILE);
      ui_dialog_text(_("Module Fatal Error"),text);
      code = EXIT_FAILURE;
      break;
    }
    
    module->reset();
    
    ++n;
  }

  newtFinished();

  return code;
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
      fprintf(logfile,_("Failed to open a NEWT window.\n"));
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

extern bool ui_window_root(struct account *data)
{
  int textbox_width = 0;
  int textbox_height = 0;
  int label1_width = 0;
  int label1_height = 0;
  int label2_width = 0;
  int label2_height = 0;
  int entry_left = 0;
  int entry_width = 0;
  int entry_height = 0;
  int next_width = 0;
  int next_height = 0;
  newtComponent textbox = 0;
  newtComponent label1 = 0;
  newtComponent entry1 = 0;
  const char *password1 = 0;
  newtComponent label2 = 0;
  newtComponent entry2 = 0;
  const char *password2 = 0;
  newtComponent next = 0;
  newtComponent form = 0;
  struct newtExitStruct es = {0};

  if(data == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(!get_text_screen_size(ROOT_TEXT,&textbox_width,&textbox_height))
    return false;

  if(!get_label_screen_size(PASSWORD_ENTER_TEXT,&label1_width,&label1_height))
    return false;

  if(!get_label_screen_size(PASSWORD_CONFIRM_TEXT,&label2_width,&label2_height))
    return false;

  entry_left = max(label1_width,label2_width) + 1;

  entry_width = NEWT_WIDTH - entry_left;

  entry_height = 1;
  
  if(!get_button_screen_size(NEXT_BUTTON_TEXT,&next_width,&next_height))
    return false;
  
  if(newtCenteredWindow(NEWT_WIDTH,NEWT_HEIGHT,ROOT_TITLE) != 0)
  {
    fprintf(logfile,_("Failed to open a NEWT window.\n"));
    return false;
  }

  textbox = newtTextbox(0,0,textbox_width,textbox_height,0);
  
  newtTextboxSetText(textbox,ROOT_TEXT);
  
  label1 = newtLabel(0,textbox_height+1,PASSWORD_ENTER_TEXT);
    
  entry1 = newtEntry(entry_left,textbox_height+1,"",entry_width,&password1,NEWT_FLAG_PASSWORD);
  
  label2 = newtLabel(0,textbox_height+label1_height+2,PASSWORD_CONFIRM_TEXT);
  
  entry2 = newtEntry(entry_left,textbox_height+label1_height+2,"",entry_width,&password2,NEWT_FLAG_PASSWORD);
  
  next = newtButton(NEWT_WIDTH-next_width,NEWT_HEIGHT-next_height,NEXT_BUTTON_TEXT);

  form = newtForm(0,0,NEWT_FLAG_NOF12);
  
  newtFormAddComponents(form,textbox,label1,entry1,label2,entry2,next,(void *) 0);
  
  while(true)
  {
    newtFormRun(form,&es);
    
    if(es.reason == NEWT_EXIT_COMPONENT && es.u.co == next)
    {
      if(get_text_length(password1) < PASSWORD_LENGTH || get_text_length(password2) < PASSWORD_LENGTH)
      {
        ui_dialog_text(PASSWORD_SHORT_TITLE,PASSWORD_SHORT_TEXT);
        continue;
      }

      if(strcmp(password1,password2) != 0)
      {
        ui_dialog_text(PASSWORD_MISMATCH_TITLE,PASSWORD_MISMATCH_TEXT);
        continue;
      }
      
      break;
    }
  }
  
  data->user = strdup("root");
  
  data->password = strdup(password1);
  
  data->group = strdup("root");
  
  data->groups = 0;
  
  data->home = strdup("/root");
  
  data->shell = strdup("/bin/bash");
  
  newtFormDestroy(form);
  
  newtPopWindow();

  return true;
}

extern bool ui_window_install(struct install *data)
{
  int textbox_width = 0;
  int textbox_height = 0;
  int next_width = 0;
  int next_height = 0;
  int checkboxtree_width = 0;
  int checkboxtree_height = 0;
  newtComponent textbox = 0;
  newtComponent next = 0;
  newtComponent checkboxtree = 0;
  int i = 0;
  struct install *grp = 0;
  newtComponent form = 0;
  struct newtExitStruct es = {0};
  bool result = true;

  if(data == 0)
  {
    errno = EINVAL;
    fprintf(logfile,"%s: %s\n",__func__,strerror(errno));
    return false;
  }

  if(!get_text_screen_size(INSTALL_TEXT,&textbox_width,&textbox_height))
    return false;

  if(!get_button_screen_size(NEXT_BUTTON_TEXT,&next_width,&next_height))
    return false;

  checkboxtree_width = NEWT_WIDTH;
  
  checkboxtree_height = NEWT_HEIGHT - textbox_height - next_height - 2;

  if(newtCenteredWindow(NEWT_WIDTH,NEWT_HEIGHT,INSTALL_TITLE) != 0)
  {
    fprintf(logfile,_("Failed to open a NEWT window.\n"));
    return false;
  }
  
  textbox = newtTextbox(0,0,textbox_width,textbox_height,0);

  newtTextboxSetText(textbox,INSTALL_TEXT);

  next = newtButton(NEWT_WIDTH-next_width,NEWT_HEIGHT-next_height,NEXT_BUTTON_TEXT);

  checkboxtree = newtCheckboxTree(0,textbox_height+1,checkboxtree_height,NEWT_FLAG_SCROLL);

  newtCheckboxTreeSetWidth(checkboxtree,checkboxtree_width);

  grp = data;

  while(grp->name != 0)
  {
    newtCheckboxTreeAddItem(checkboxtree,grp->name,&grp->checked,0,i,NEWT_ARG_LAST);
    newtCheckboxTreeSetEntryValue(checkboxtree,&grp->checked,(grp->checked) ? '*' : ' ');
    ++i;
    ++grp;
  }

  form = newtForm(0,0,NEWT_FLAG_NOF12);

  newtFormAddComponents(form,textbox,next,checkboxtree,(void *) 0);

  newtFormSetCurrent(form,checkboxtree);

  while(true)
  {
    newtFormRun(form,&es);
    
    if(es.reason == NEWT_EXIT_COMPONENT && es.u.co == next)
    {
      grp = data;

      while(grp->name != 0)
      {
        if(strcmp(grp->name,"base") == 0)
          break;
        ++grp;
      }

      if(grp != 0 && newtCheckboxTreeGetEntryValue(checkboxtree,&grp->checked) != '*')
      {
        ui_dialog_text(NO_BASE_TITLE,NO_BASE_TEXT);
        continue;
      }

      result = true;

      break;
    }
  }

  grp = data;

  while(grp->name != 0)
  {
    grp->checked = (newtCheckboxTreeGetEntryValue(checkboxtree,&grp->checked) == '*');
    ++grp;
  }

  newtFormDestroy(form);

  newtPopWindow();

  return result;
}

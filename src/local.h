#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <wchar.h>
#include <errno.h>
#include <limits.h>
#include "text.h"

#define LOGFILE "fwsetup.log"
#define INSTALL_ROOT "/home/vniklos/fwsetup-ng/root"
#define KIBIBYTE (1LL << 10LL)
#define MEBIBYTE (1LL << 20LL)
#define GIBIBYTE (1LL << 30LL)
#define TEBIBYTE (1LL << 40LL)

struct global
{
  bool netinstall;
};

struct install
{
  char *name;
  bool checked;
};

struct account
{
  char *user;
  char *password;
  char *group;
  char *groups;
  char *home;
  char *shell;
};

struct module
{
  bool (*run) (void);
  void (*reset) (void);
  const char *name;
};

extern bool mkdir_recurse(const char *path);
extern bool size_to_string(char *s,size_t n,long long size,bool pad);
extern int get_text_length(const char *s);
extern int get_text_screen_width(const char *s);
extern bool get_text_screen_size(const char *text,int *width,int *height);
extern bool get_button_screen_size(const char *text,int *width,int *height);
extern bool get_label_screen_size(const char *text,int *width,int *height);
static inline long min(long a,long b) { return (a < b) ? a : b; }
static inline long max(long a,long b) { return (a > b) ? a : b; }
extern int ui_main(int argc,char **argv);
extern void ui_dialog_text(const char *title,const char *text);
extern bool ui_dialog_yesno(const char *title,const char *text,bool defaultno);
extern bool ui_dialog_progress(const char *title,const char *text,int percent);
extern bool ui_window_root(const char *title,const char *text,struct account *data);
extern bool ui_window_install(const char *title,struct install *groups);
extern FILE *logfile;
extern int main(int argc,char **argv);

extern struct global g;
extern struct module install_module;
extern struct module *modules[];

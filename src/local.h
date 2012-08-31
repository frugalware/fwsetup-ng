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
#define INSTALL_ROOT "/home/ryuo/fwsetup-ng/root"
#define KIBIBYTE (1LL << 10LL)
#define MEBIBYTE (1LL << 20LL)
#define GIBIBYTE (1LL << 30LL)
#define TEBIBYTE (1LL << 40LL)

typedef bool (*ui_dialog_progress_callback) (char *text,size_t n,int *percent,void *data);

struct pkggroup
{
  const char *name;
  bool checked;
};

struct install
{
  bool update_databases;
  struct pkggroup *pkgs;
};

struct dldata
{
  char eta[9];
  char rate[47];
  char size[20];
  char size_perc[5];
  int size_perc_int;
  char pkg[12];
  char pkg_perc[5];
  int pkg_perc_int;
  char *file;
};

struct module
{
  int (*run) (void);
  void (*reset) (void);
  const char *name;
};

extern bool mkdir_recurse(const char *path);
extern bool size_to_string(char *s,size_t n,long long size);
extern int get_text_screen_width(const char *s);
extern bool get_text_screen_size(const char *text,int *width,int *height);
extern bool get_button_screen_size(const char *text,int *width,int *height);
static inline long min(long a,long b) { return (a < b) ? a : b; }
static inline long max(long a,long b) { return (a > b) ? a : b; }
extern int ui_main(int argc,char **argv);
extern void ui_dialog_text(const char *title,const char *text);
extern bool ui_dialog_yesno(const char *title,const char *text,bool defaultno);
extern bool ui_dialog_progress(const char *title,ui_dialog_progress_callback cb,void *data);
extern int ui_window_install(const char *title,struct install *data);
extern bool ui_dialog_progress_install(const char *title,const struct dldata *data);
extern FILE *logfile;
extern int main(int argc,char **argv);

extern struct module module_install;

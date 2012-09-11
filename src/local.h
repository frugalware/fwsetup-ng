#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <wchar.h>
#include <errno.h>
#include <limits.h>
#include <parted/parted.h>
#include "text.h"

#define LOGFILE "fwsetup.log"
#define INSTALL_ROOT "/home/vniklos/fwsetup-ng/root"
#define KIBIBYTE (1LL << 10LL)
#define MEBIBYTE (1LL << 20LL)
#define GIBIBYTE (1LL << 30LL)
#define TEBIBYTE (1LL << 40LL)

struct global
{
  PedDiskType *doslabel;
  PedDiskType *gptlabel;
  bool netinstall;
};

struct parted
{
  PedDevice *device;
  PedConstraint *constraint;
  PedDisk *disk;
  bool modified;
};

struct install
{
  char *name;
  bool checked;
};

struct account
{
  char *name;
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
extern bool execute(const char *command,const char *root,pid_t *cpid);
extern void *malloc0(size_t size);
extern struct parted *parted_open(const char *path);
extern bool parted_new_disk_label(struct parted *parted,PedDiskType *type);
static inline bool is_normal_partition(PedPartitionType type)
{
  switch(type)
  {
    case PED_PARTITION_NORMAL:
    case PED_PARTITION_LOGICAL:
    case PED_PARTITION_EXTENDED:
      return true;

    case PED_PARTITION_FREESPACE:
    case PED_PARTITION_METADATA:
    case PED_PARTITION_PROTECTED:
      return false;
    
    default:
      return false;    
  }
}
extern bool parted_new_partition(struct parted *parted,const char *size);
extern bool parted_delete_last_partition(struct parted *parted);
extern void parted_close(struct parted *parted);
extern int get_text_screen_width(const char *s);
extern bool get_text_screen_size(const char *text,int *width,int *height);
extern bool get_button_screen_size(const char *text,int *width,int *height);
extern bool get_label_screen_size(const char *text,int *width,int *height);
extern bool get_checkbox_screen_size(const char *text,int *width,int *height);
static inline long min(long a,long b) { return (a < b) ? a : b; }
static inline long max(long a,long b) { return (a > b) ? a : b; }
static inline long minv(long *v,size_t size)
{
  long i = 0;
  
  for( size_t n = 0 ; n < size ; ++n )
    i = min(i,v[n]);
  
  return i;
}
static inline long maxv(long *v,size_t size)
{
  long i = 0;
  
  for( size_t n = 0 ; n < size ; ++n )
    i = max(i,v[n]);
  
  return i;
}
extern int ui_main(int argc,char **argv);
extern void ui_dialog_text(const char *title,const char *text);
extern bool ui_dialog_yesno(const char *title,const char *text,bool defaultno);
extern bool ui_dialog_progress(const char *title,const char *text,int percent);
extern bool ui_window_root(struct account *data);
extern bool ui_window_user(struct account *data);
extern bool ui_window_time(char **data,char **zone,bool *utc);
extern bool ui_window_install(struct install *groups);
extern FILE *logfile;
extern int main(int argc,char **argv);

extern struct global *g;
extern struct module install_module;
extern struct module postconfig_module;
extern struct module *modules[];

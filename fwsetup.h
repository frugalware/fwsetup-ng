#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <fcntl.h>
#include <assert.h>
#include <wchar.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/major.h>
#include <limits.h>
#include <errno.h>
#include <blkid.h>

#define _(S) S
#define memzero(P,N) memset(P,0,N)
#define assert_not_reached() assert(0)
#define VIRTBLK_MAJOR 253
#define DM_MAJOR 254
#define LOGFILE "fwsetup.log"
#define LOG_ERRNO() eprintf("%s: %s\n",__func__,strerror(errno))
#define ASSERT_ARGS(B,R) if(B) { errno = EINVAL; LOG_ERRNO(); return R; } 
#define EXECUTE_START_TEXT _("About to execute command '%s'.\n")
#define EXECUTE_STOP_TEXT _("Successfully executed command '%s'.\n")
#define PROCESS_EXIT_ERROR_TEXT _("A process (%d) has exitted with a non-zero exit code (%d).\n")
#define WINDOWTITLE_TEXT _("Frugalware Linux Installer")
#define NEXTBUTTON_TEXT _("Next")
#define PREVIOUSBUTTON_TEXT _("Previous")

#ifdef NEWT
#include <newt.h>

#define TEXTSIZE_TEXT _("Failed to retrieve a text block's screen dimensions.\n")
#define NEWTWINDOW_TEXT _("Failed to open a NEWT window.\n")
#define NEWTINIT_TEXT _("Failed to initialize NEWT.\n")

struct database
{
  char *locale;
  int screen_width;
  int screen_height;
  int window_width;
  int window_height;
  int window_x;
  int window_y;
};

extern bool get_text_size(const char *text,int *width,int *height);
extern bool get_button_size(const char *text,int *width,int *height);
#else
#error "No known user interface is defined."
#endif

enum order
{
  ORDER_NONE,
  ORDER_ERROR,
  ORDER_PREVIOUS,
  ORDER_NEXT
};

enum devicetype
{
  DEVICETYPE_IDE,
  DEVICETYPE_SCSI,
  DEVICETYPE_VIRTIO,
  DEVICETYPE_MDADM
};

struct module
{
  const char *name;
  enum order (*run) (struct database *);
};

struct string
{
  struct string *prev;
  struct string *next;
  char *data;
};

struct partition
{
  struct partition *prev;
  struct partition *next;
  unsigned long long num;
  char *name;
  char *uuid;
  unsigned long long start;
  unsigned long long end;
  unsigned long long sectors;
  unsigned char type_n;
  char *type_s;
  unsigned long long flags;
  bool primary;
  bool extended;
  bool logical;
};

struct device
{
  struct device *prev;
  struct device *next;
  char *path;
  unsigned long long sector_size;
  enum devicetype type;
  char *label;
  char *uuid;
  struct partition *partitions;
};

extern void *malloc0(size_t n);
extern pid_t execute(const char *cmd);
extern void eprintf(const char *fmt,...) __attribute__((format(printf,1,2)));
extern void snprintf_append(char *s,size_t size,const char *fmt,...) __attribute__((format(printf,3,4)));
extern void *list_append(void *list,size_t n);
extern void *list_find_start(void *list);
extern void *list_find_end(void *list);
extern void list_free(void *list,void (*cb) (void *));
extern void free_string(void *p);
extern void free_partition(void *p);
extern struct device *read_device_data(const char *path);
extern bool write_device_data(const struct device *device);
extern void free_device(struct device *device);
extern int main(void);
extern struct module begin_module;
extern struct module partition_setup_module;
extern struct module end_module;

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

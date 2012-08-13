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
#include <linux/major.h>
#include <stdint.h>
#include <blkid.h>

#define _(S) S
#define memzero(P,N) memset(P,0,N)
#define assert_not_reached() assert(0)
#define VIRTBLK_MAJOR 253
#define DM_MAJOR 254
#define LOGFILE "fwsetup.log"
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
  char *name;
  uint64_t start;
  uint64_t end;
  uint64_t sectors;
  uint16_t type;
};

struct device
{
  struct device *prev;
  struct device *next;
  char *path;
  uint64_t sector_size;
  enum devicetype type;
  char *label;
};

extern void *malloc0(size_t n);
extern void eprintf(const char *s,...) __attribute__((format(printf,1,2)));;
extern void *list_append(void *list,size_t n);
extern void *list_find_start(void *list);
extern void *list_find_end(void *list);
extern void list_free(void *list,void (*cb) (void *));
extern void string_free(void *string);
extern struct device *read_device_data(const char *path);
extern void free_device(struct device *device);
extern int main(void);
extern struct module begin_module;
extern struct module partition_setup_module;
extern struct module end_module;

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

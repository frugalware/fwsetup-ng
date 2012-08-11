#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <assert.h>
#include <wchar.h>

#define _(S) S
#define memzero(P,N) memset(P,0,N)
#define WINDOW_TITLE _("Frugalware Linux Installer")

#ifdef NEWT
#include <newt.h>

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

struct module
{
  const char *name;
  enum order (*run) (struct database *);
};

extern void eprintf(const char *s,...) __attribute__((format(printf,1,2)));;
extern int main(void);

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

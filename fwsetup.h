#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <assert.h>

#ifdef NEWT
#include <newt.h>

struct database
{
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

struct module
{
  bool (*run) (struct database *);
};

extern void eprintf(const char *s,...) __attribute__((format(printf,1,2)));;
extern int main(void);

// -%- strip: yes; add-newline: yes; use-tabs: no; indent-width: 2; tab-width: 2; -%-

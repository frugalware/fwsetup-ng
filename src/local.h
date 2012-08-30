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

#define _(S) S
#define LOGFILE "fwsetup.log"
#define INSTALL_ROOT "/home/ryuo/fwsetup-ng/root"
#define NEWT_WIDTH  70
#define NEWT_HEIGHT 21
#define KIBIBYTE (1LL << 10LL)
#define MEBIBYTE (1LL << 20LL)
#define GIBIBYTE (1LL << 30LL)
#define TEBIBYTE (1LL << 40LL)

extern bool mkdir_recurse(const char *path);
extern bool size_to_string(char *s,size_t n,long long size);
extern int get_text_screen_width(const char *s);
extern bool get_text_screen_size(const char *text,int *width,int *height);
extern int ui_main(int argc,char **argv);
extern FILE *logfile;
extern int main(int argc,char **argv);

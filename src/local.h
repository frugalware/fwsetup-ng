#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>

#define _(S) S
#define LOGFILE "fwsetup.log"
#define INSTALL_ROOT "/home/ryuo/fwsetup-ng/root"
#define NEWT_WIDTH  70
#define NEWT_HEIGHT 21

extern bool mkdir_recurse(const char *path);
extern int ui_main(int argc,char **argv);
extern FILE *logfile;
extern int main(int argc,char **argv);

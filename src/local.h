#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>
#include <limits.h>

#define _(S) S
#define LOGFILE "fwsetup.log"
#define INSTALL_ROOT "/home/ryuo/fwsetup-ng/root"
#define NEWT_WIDTH  70
#define NEWT_HEIGHT 21
#define KIBIBYTE (1LLU << 10LLU)
#define MEBIBYTE (1LLU << 20LLU)
#define GIBIBYTE (1LLU << 30LLU)
#define TEBIBYTE (1LLU << 40LLU)

extern bool mkdir_recurse(const char *path);
extern int ui_main(int argc,char **argv);
extern FILE *logfile;
extern int main(int argc,char **argv);

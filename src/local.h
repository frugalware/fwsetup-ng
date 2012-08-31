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
#define OK_BUTTON_TEXT _("Ok")
#define CANCEL_BUTTON_TEXT _("Cancel")
#define YES_BUTTON_TEXT _("Yes")
#define NO_BUTTON_TEXT _("No")
#define NEWT_WIDTH  70
#define NEWT_HEIGHT 21
#define KIBIBYTE (1LL << 10LL)
#define MEBIBYTE (1LL << 20LL)
#define GIBIBYTE (1LL << 30LL)
#define TEBIBYTE (1LL << 40LL)

typedef bool (*ui_dialog_progress_callback) (char *text,size_t n,int *percent,void *data);

extern bool mkdir_recurse(const char *path);
extern bool size_to_string(char *s,size_t n,long long size);
extern int get_text_screen_width(const char *s);
extern bool get_text_screen_size(const char *text,int *width,int *height);
extern bool get_button_screen_size(const char *text,int *width,int *height);
extern int ui_main(int argc,char **argv);
extern void ui_dialog_text(const char *title,const char *text);
extern bool ui_dialog_yesno(const char *title,const char *text,bool defaultno);
extern bool ui_dialog_progress(const char *title,ui_dialog_progress_callback cb,void *data);
extern FILE *logfile;
extern int main(int argc,char **argv);

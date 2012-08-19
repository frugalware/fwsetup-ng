#pragma once

#include <sys/types.h>
#include <string>

using std::string;

pid_t execute(const string &cmd);

#ifdef NEWT
bool get_text_size(const string &text,int &width,int &height);
#endif

#pragma once

#include <sys/types.h>
#include <string>

#define KIBIBYTE (2LLU <<  9LLU)
#define MEBIBYTE (2LLU << 19LLU)
#define GIBIBYTE (2LLU << 29LLU)
#define TEBIBYTE (2LLU << 39LLU)

using std::string;

pid_t execute(const string &cmd);
unsigned long long string_to_size(const string &text);
string size_to_string(unsigned long long n);

#ifdef NEWT
bool get_text_size(const string &text,int &width,int &height);
bool get_button_size(const string &text,int &width,int &height);
#endif

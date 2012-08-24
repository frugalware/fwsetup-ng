#pragma once

#include <sys/types.h>
#include <string>
#include <fstream>

#define KIBIBYTE (2LLU <<  9LLU)
#define MEBIBYTE (2LLU << 19LLU)
#define GIBIBYTE (2LLU << 29LLU)
#define TEBIBYTE (2LLU << 39LLU)

using std::string;
using std::ofstream;

extern ofstream logfile;

pid_t execute(const string &cmd);
unsigned long long string_to_size(const string &text);
string size_to_string(unsigned long long n);

#pragma once

#include <sys/types.h>
#include <string>
#include <fstream>

#define KIBIBYTE (1LLU << 10LLU)
#define MEBIBYTE (1LLU << 20LLU)
#define GIBIBYTE (1LLU << 30LLU)
#define TEBIBYTE (1LLU << 40LLU)

using std::string;
using std::ofstream;

extern ofstream logfile;

pid_t execute(const string &cmd);
bool zapLabel(const string &path);
unsigned long long string_to_size(const string &text);
string size_to_string(unsigned long long n);

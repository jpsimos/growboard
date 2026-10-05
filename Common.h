/*
 * author Jacob A Psimos
 */

#ifndef __COMMON_H
#define __COMMON_H

#include <stdarg.h>

// Terminal escaped string representations of arrow keys.
#define TERM_ARROW_UP "\x1b\x5b\x41"
#define TERM_ARROW_DOWN "\x1b\x5b\x42"
#define TERM_ARROW_LEFT "\x1b\x5b\x44"
#define TERM_ARROW_RIGHT "\x1b\x5b\x43"

namespace IO
{
  extern Stream* InOutStream;
}

namespace Text
{
  ssize_t ReadLine(char input[], ssize_t maximum, Stream* stream);
  bool streq(const char *a, const char *b);
  char* stripNewLine(char str[]);
}

namespace Numeric
{
  bool CompareDouble(const double a, const double b, const double epsilon = 0.0001);
}

#endif

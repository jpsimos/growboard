/*
 * author Jacob A Psimos
 */

#ifndef __COMMON_H
#define __COMMON_H

#include <stdarg.h>
#include <cstddef>

// Terminal escaped string representations of arrow keys.
#define TERM_ARROW_UP "\x1b\x5b\x41"
#define TERM_ARROW_DOWN "\x1b\x5b\x42"
#define TERM_ARROW_LEFT "\x1b\x5b\x44"
#define TERM_ARROW_RIGHT "\x1b\x5b\x43"

namespace Text
{
  size_t ReadLine(char input[], const size_t maxLengthInclTerm);
  bool CStrEq(const char *a, const char *b, const bool caseSensitive = true);
  char* CStrStripNewLine(char str[]);
  char* CStrLStrip(char str[]);
}

#endif

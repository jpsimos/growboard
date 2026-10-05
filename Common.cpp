/*
 * author Jacob A Psimos
 */

#include <elapsedMillis.h>
#include "Common.h"

namespace IO
{
  Stream* InOutStream = &Serial;
}

namespace Text
{
  ssize_t ReadLine(char input[], ssize_t maxLengthInclTerm, Stream* stream)
  {
    ssize_t num = 0;
    elapsedMillis timeout;
    do
    {
      if(stream->available())
      {
        char in = static_cast<char>(stream->read());
        if('\n' != in && '\r' != in)
        {
          input[num++] = in;
        }
        else
        {
          break;
        }
        timeout = 0UL;
      }
      else if(timeout >= 5)
      {
        num = 0;
        break;
      }
    }
    while(num < maxLengthInclTerm - 1);
    input[num] = '\0';
    return num;
  }
  
  bool streq(const char *a, const char *b)
  {
    bool equal = static_cast<bool>(a != b && a != NULL && b != NULL);
  	while(equal && a != b)
  	{
      if(*a != *b)
      {
        equal = false;
      }
  		else if(*a++ == '\0' || *b++ == '\0')
  		{
  			break;
  		}
  	}
    return equal;
  }
  
  char* stripNewLine(char str[])
  {
    char* nl = strchr(str, '\r');
    if(nl != NULL)
    {
      *nl = '\0';
    }
    nl = strchr(str, '\n');
    if(nl != NULL)
    {
      *nl = '\0';
    }
    return str;
  }
} // namespace Text

namespace Numeric
{
  bool CompareDouble(const double a, const double b, const double epsilon)
  {
    return static_cast<bool>(fabs(fabs(a) - fabs(b)) <= epsilon);
  }
  
} // namespace Numeric

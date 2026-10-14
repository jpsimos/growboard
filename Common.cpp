/*
 * author Jacob A Psimos
 */

#include <elapsedMillis.h>
#include "Common.h"

namespace Text
{
  size_t ReadLine(char input[], const size_t maxLengthInclTerm)
  {
    size_t num = 0;
    char in;
    elapsedMillis timeout;
    do
    {
      if(Serial.available())
      {
        in = static_cast<char>(Serial.read());
        
        if('\n' != in && '\r' != in)
        {
          if(input)
          {
            if(num < maxLengthInclTerm)
            {
              input[num++] = in;
            }
            else
            {
              break;
            }
          }
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
      else
      {
        yield();
      }
    }
    while(num < maxLengthInclTerm - 1);
    input[num] = '\0';
    return num;
  }
  
  bool CStrEq(const char *a, const char *b, const bool caseSensitive)
  {
    bool equal = static_cast<bool>(a != b && a != NULL && b != NULL);
  	while(equal && a != b)
  	{
      if(caseSensitive ? (*a != *b) : (tolower(*a) != tolower(*b)))
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
  
  char* CStrStripNewLine(char str[])
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

  char* CStrLStrip(char str[])
  {
    char* ret = nullptr;
    
    if(str)
    {
      ret = &str[0];
      
      while(*ret != '\0' && isblank(*ret))
      {
         ret = ret + 1;
      }
    }
    
    return ret;
  }
} // namespace Text

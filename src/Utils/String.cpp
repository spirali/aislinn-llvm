
#include "String.h"

#include <stdlib.h>

size_t parseSizeString(const std::string &Str)
{
  char *End;
  size_t Value = strtol(Str.c_str(), &End, 10);

  switch (*End) {
    case 'G':
      Value *= 1024;
    case 'M':
      Value *= 1024;
    case 'K':
      Value *= 1024;
    case 0:
      return Value;
    default:
      return 0;
  }
}

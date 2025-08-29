#include <stdarg.h>
#include <stdio.h>
#include "StrUtils.h"

ExString strFormat(const char *fmt, ...) {
  va_list args1, args2;
  ExString result;
  int len;

  va_start(args1, fmt);
  va_copy(args2, args1);
  len = vsnprintf(NULL, 0, fmt, args2);
  va_end(args2);
  if (len > 0) {
    if (result.reserve(len + 1)) { // ???
      vsnprintf(result.begin(), len + 1, fmt, args1);
      result.setLen(len);
    }
  }
  va_end(args1);
  return result;
}

#pragma once

#include <WString.h>

class ExString : public String {
public:
  void setLen(int len) {
    String::setLen(len);
  }
};

ExString strFormat(const char *fmt, ...);

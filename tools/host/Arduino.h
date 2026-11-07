// Minimal Arduino stand-in so the SCPI core and Vrekrer parser build on the host.
#pragma once
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t byte;
class __FlashStringHelper;
#define F(x) reinterpret_cast<const __FlashStringHelper *>(x)
#define strcpy_P strcpy
inline unsigned long millis() { return 0; }
#define HEX 16
struct String {  // only for the library's debug printer
  char b[64];
  String(const char *s) { snprintf(b, sizeof b, "%s", s); }
};

class Stream {
 public:
  virtual ~Stream() {}
  virtual int available() = 0;
  virtual int read() = 0;
  virtual int peek() = 0;
  virtual size_t write(uint8_t) = 0;
  virtual void flush() {}
  size_t print(const char *s) { size_t n = 0; while (*s) { write(*s++); n++; } return n; }
  size_t print(const __FlashStringHelper *s) { return print((const char *)s); }
  size_t print(int v) { char b[16]; snprintf(b, sizeof b, "%d", v); return print(b); }
  size_t print(unsigned long v, int = 10) { return print((int)v); }
  size_t print(const String &s) { return print(s.b); }
  size_t println(const char *s) { size_t n = print(s); write('\n'); return n + 1; }
  size_t println(const __FlashStringHelper *s) { return println((const char *)s); }
  size_t println(int v) { size_t n = print(v); write('\n'); return n + 1; }
  size_t println(const String &s) { return println(s.b); }
  size_t println() { write('\n'); return 1; }
};

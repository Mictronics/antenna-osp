// Host check of the SCPI core: g++ -std=c++17 -Isrc tools/scpi_test.cpp src/scpi.cpp -o /tmp/scpi_test && /tmp/scpi_test
#ifndef FW_VERSION
#define FW_VERSION "0.0.0"
#endif
#include "scpi.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>

static uint8_t relays;
static bool remote;
void remote_set(bool on) { remote = on; }
static uint8_t hc;
uint8_t theme_get() { return hc; }
void theme_set(uint8_t t) { hc = t; }
uint8_t relays_get() { return relays; }
void relays_apply(uint8_t mask, uint8_t val) { relays = (relays & ~mask) | (val & mask); }

static std::string store;
void paths_write(const char *s) { store = s; }
size_t paths_read(char *b, size_t cap) { size_t n = std::min(store.size(), cap - 1); memcpy(b, store.data(), n); b[n] = 0; return n; }

static std::string got;
static void sink(const char *s) { got += s; }
static std::string ask(const char *line) { got.clear(); scpi_handle(line, sink); return got; }

#define EQ(line, want) do { auto r = ask(line); if (r != want) { printf("FAIL %s -> '%s' want '%s'\n", line, r.c_str(), want); return 1; } } while (0)

int main() {
  EQ("*IDN?", "DIY,OSP-EMU,000001," FW_VERSION "\n");
  EQ("*OPC?", "1\n"); assert(remote);
  EQ("SYST:LOC", ""); assert(!remote);
  EQ("*OPC?", "1\n"); assert(remote);
  EQ("DISP:THEM HCON", ""); assert(hc == 1); EQ("DISP:THEM?", "HCON\n");
  EQ("DISP:THEM LIGH", ""); assert(hc == 2); EQ("DISP:THEM?", "LIGH\n");
  EQ("DISP:THEM NORM", ""); assert(!hc); EQ("DISP:THEM?", "NORM\n");
  EQ("DISP:THEM foo", ""); EQ("SYST:ERR?", "-224,\"Illegal parameter value\"\n");
  EQ("SYST:ERR?", "0,\"No error\"\n");

  EQ("ROUT:CLOS (@F01M01(0112))", ""); assert(relays == 0b00010);
  EQ("ROUT:CLOS? (@F01M01(0112))", "1\n");
  EQ("ROUT:CLOS? (@F01M01(0012))", "0\n");
  EQ("ROUT:CLOS? (@F01M01(0012)),VAL", "1\n");
  EQ("ROUT:CLOS? (@F01M01(0112,0113)),SUMM", "0\n");
  EQ("rout:close (@F01M01(0101:0103))", ""); assert(relays == 0b00111);  // 01..06 numbering, range
  EQ("ROUT:CLOS (@F01M01(0001,0005,0116))", ""); assert(relays == 0b100110);
  EQ("*RST", ""); assert(relays == 0);
  EQ("*RST;ROUT:CLOS (@F01M01(0111))", ""); assert(relays == 0b00001);

  EQ("ROUT:PATH:DEF \"Path A\",(@F01M01(0111,0012,0114))", "");
  EQ("ROUT:PATH:DEF? \"Path A\"", "(@F01M01(0111,0012,0114))\n");
  EQ("*RST", "");
  EQ("ROUT:PATH:CAT?", "\"Path A\"\n");
  EQ("ROUT:PATH:CAT? ACT", "\"\"\n");
  EQ("ROUT:CLOS \"Path A\"", ""); assert(relays == 0b01001);
  EQ("ROUT:CLOS? \"Path A\"", "1,1,1\n");
  EQ("ROUT:PATH:CAT? ACT", "\"Path A\"\n");
  EQ("ROUT:PATH:LAST?", "\"Path A\"\n");
  EQ("ROUT:CLOS (@F01M01(0113))", "");
  EQ("ROUT:PATH:LAST?", "<Individual Settings>\n");
  EQ("ROUT:PATH:DEF \"B\",(@F01M01(0112))", "");
  EQ("ROUT:CLOS:COMB \"Path A\", 'B'", ""); assert(relays == 0b01111);  // K13 untouched by the paths
  EQ("ROUT:PATH:DEL \"Path A\",\"B\"", "");
  EQ("ROUT:PATH:CAT?", "\"\"\n");

  EQ("ROUT:CLOS (@F02M01(0101))", "");
  EQ("SYST:ERR?", "-200,\"Execution error;module not present\"\n");
  EQ("FOO:BAR", "");
  EQ("ROUT:CLOS (@F01M01(0299))", "");
  EQ("SYST:ERR:ALL?", "-113,\"Undefined header\",-222,\"Data out of range\"\n");
  EQ("ROUT:CLOS \"nope\"", "");
  EQ("*STB?", "4\n");
  EQ("*ESR?", "48\n");
  EQ("DIAG:SERV:HWIN?", "\"F01|OSP230|000001|0000.0000.00|0|01.00\",\"F01M00|OSPMAINBOARD|000001|0000.0000.00|0|01.00\","
                        "\"F01M01|OSP-B101|000001|0000.0000.00|0|01.00\"\n");
  // remaining commands and the comma/semicolon protection
  EQ("*CLS", ""); EQ("SYST:ERR?", "0,\"No error\"\n");
  EQ("*ESE 32", ""); EQ("*ESE?", "32\n");
  EQ("*SRE 8", ""); EQ("*SRE?", "8\n");
  EQ("*TST?", "0\n"); EQ("*OPC", ""); EQ("*ESR?", "1\n");
  EQ("SYST:VERS?", "1999.0\n");
  EQ("ROUT:CLOS (@F01M01(0111))", ""); EQ("SYST:PRES", ""); assert(relays == 0);
  EQ("ROUT:PATH:MULT ON", ""); EQ("ROUT:PATH:MULT?", "1\n"); EQ("ROUT:PATH:MULT OFF", ""); EQ("ROUT:PATH:MULT?", "0\n");
  EQ("ROUT:PATH \"a,b;c\",(@F01M01(0111))", "");  // PATH without :DEF, odd name
  EQ("ROUT:PATH:CAT?", "\"a,b;c\"\n");
  EQ("ROUT:CLOS \"a,b;c\"", ""); assert(relays == 1);
  EQ("ROUT:PATH:DEL:NAME \"a,b;c\"", ""); EQ("ROUT:PATH:CAT?", "\"\"\n");
  EQ("ROUT:PATH:DEF \"x\",(@F01M01(0111))", ""); EQ("ROUT:PATH:DEL:ALL", ""); EQ("ROUT:PATH:CAT?", "\"\"\n");
  EQ("ROUT:CLOS? (@F01M01(0111,0112,0113,0114,0115,0116,0101,0102,0103,0104,0105,0106))", "1,0,0,0,0,0,1,0,0,0,0,0\n"); // 12 items; K11 is still on
  EQ("ROUT:CLOS? (@F01M01(0111))", "1\n");
  // persistence: saved on change, restored by scpi_load_paths()
  EQ("ROUT:PATH:DEF \"A\",(@F01M01(0111,0012))", ""); assert(store == "03 01 A\n");
  EQ("ROUT:PATH:DEF \"B x\",(@F01M01(0113))", ""); assert(store == "03 01 A\n04 04 B x\n");
  EQ("ROUT:PATH:DEL \"A\"", ""); assert(store == "04 04 B x\n");
  EQ("ROUT:PATH:DEL:ALL", ""); assert(store == "");
  store = "03 01 A\n04 04 B x\nzz\n3f ff C\n";  // third line is junk, fourth has val outside mask
  scpi_load_paths();
  EQ("ROUT:PATH:CAT?", "\"A\",\"B x\",\"C\"\n");
  EQ("ROUT:PATH:DEF? \"C\"", "(@F01M01(0111,0112,0113,0114,0115,0116))\n");
  puts("scpi ok");
}

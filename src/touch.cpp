#include "touch.h"
#include <Wire.h>

static const uint8_t ADDR = 0x38, PIN_RST = 24;

void touch_init() {
  Wire1.setSDA(34);
  Wire1.setSCL(35);
  Wire1.setClock(400000);
  Wire1.begin();
  pinMode(PIN_RST, OUTPUT);
  digitalWrite(PIN_RST, HIGH); delay(10);
  digitalWrite(PIN_RST, LOW);  delay(10);
  digitalWrite(PIN_RST, HIGH); delay(300);
}

bool touch_read(uint16_t &x, uint16_t &y) {
  uint8_t b[5];
  Wire1.beginTransmission(ADDR);
  Wire1.write((uint8_t)0x02);  // TD_STATUS, then touch 1 X/Y
  if (Wire1.endTransmission(false)) return false;
  if (Wire1.requestFrom(ADDR, (uint8_t)5) != 5) return false;
  for (auto &v : b) v = Wire1.read();
  if ((b[0] & 0x0F) == 0) return false;
  // Panel is natively 320x480 portrait; rotate to match MADCTL 0x28 (flip with 0xE8: x = 479-ty, y = tx).
  uint16_t tx = ((b[1] & 0x0F) << 8) | b[2], ty = ((b[3] & 0x0F) << 8) | b[4];
  x = ty;
  y = 319 - tx;
  return true;
}

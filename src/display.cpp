#include "display.h"
#include <SPI.h>

static const uint8_t PIN_RST = 23, PIN_DC = 20, PIN_BL = 22, PIN_CS = 21;
static const SPISettings LCD_SPI(50000000, MSBFIRST, SPI_MODE0);

static void cmd(uint8_t c) {
  digitalWrite(PIN_DC, LOW);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(c);
  digitalWrite(PIN_CS, HIGH);
}

static void data(std::initializer_list<uint8_t> d) {
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  for (uint8_t b : d) SPI.transfer(b);
  digitalWrite(PIN_CS, HIGH);
}

static void window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  uint16_t x1 = x + w - 1, y1 = y + h - 1;
  cmd(0x2A);
  data({(uint8_t)(x >> 8), (uint8_t)x, (uint8_t)(x1 >> 8), (uint8_t)x1});
  cmd(0x2B);
  data({(uint8_t)(y >> 8), (uint8_t)y, (uint8_t)(y1 >> 8), (uint8_t)y1});
  cmd(0x2C);
}

void display_init() {
  for (uint8_t p : {PIN_RST, PIN_DC, PIN_BL, PIN_CS}) pinMode(p, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  // Write-only display: MISO is unused, but the core still claims the default RX pin.
  SPI.setSCK(18);
  SPI.setTX(19);
  SPI.begin();
  SPI.beginTransaction(LCD_SPI);

  digitalWrite(PIN_RST, HIGH); delay(100);
  digitalWrite(PIN_RST, LOW);  delay(100);
  digitalWrite(PIN_RST, HIGH); delay(100);

  // Init sequence from the Waveshare example (LCD_3in5.cpp).
  cmd(0x11); delay(120);
  cmd(0x36); data({0x28});  // landscape (MV), BGR; try 0xE8 if upside down
  cmd(0x3A); data({0x05});  // 16 bit/pixel
  cmd(0xF0); data({0xC3});
  cmd(0xF0); data({0x96});
  cmd(0xB4); data({0x01});
  cmd(0xB7); data({0xC6});
  cmd(0xC0); data({0x80, 0x45});
  cmd(0xC1); data({0x13});
  cmd(0xC2); data({0xA7});
  cmd(0xC5); data({0x0A});
  cmd(0xE8); data({0x40, 0x8A, 0x00, 0x00, 0x29, 0x19, 0xA5, 0x33});
  cmd(0xE0); data({0xD0, 0x08, 0x0F, 0x06, 0x06, 0x33, 0x30, 0x33, 0x47, 0x17, 0x13, 0x13, 0x2B, 0x31});
  cmd(0xE1); data({0xD0, 0x0A, 0x11, 0x0B, 0x09, 0x07, 0x2F, 0x33, 0x47, 0x38, 0x15, 0x16, 0x2C, 0x32});
  cmd(0xF0); data({0x3C});
  cmd(0xF0); data({0x69});
  delay(120);
  cmd(0x21);  // inversion on
  cmd(0x29);  // display on
  digitalWrite(PIN_BL, HIGH);
}

void display_blit(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buf) {
  window(x, y, w, h);
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(buf, nullptr, (size_t)w * h * 2);  // tx-only: keeps buf intact
  digitalWrite(PIN_CS, HIGH);
}

void display_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c) {
  static uint8_t row[LCD_W * 2];
  for (uint16_t i = 0; i < w; i++) { row[2 * i] = c >> 8; row[2 * i + 1] = c; }
  window(x, y, w, h);
  digitalWrite(PIN_DC, HIGH);
  digitalWrite(PIN_CS, LOW);
  for (uint16_t j = 0; j < h; j++) SPI.transfer(row, nullptr, (size_t)w * 2);
  digitalWrite(PIN_CS, HIGH);
}

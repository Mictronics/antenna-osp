#pragma once
#include <Arduino.h>

constexpr uint16_t LCD_W = 320, LCD_H = 480;

void display_init();
// data: big-endian RGB565, w*h pixels, sent to window at (x,y).
void display_blit(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *data);
void display_fill(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565);

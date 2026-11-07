#pragma once
#include <Arduino.h>

void touch_init();
// True while a finger is down; x,y in panel coordinates.
bool touch_read(uint16_t &x, uint16_t &y);

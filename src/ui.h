#pragma once
#include <Arduino.h>

// Implemented in main.cpp, shared with the SCPI handler.
void route_set(uint8_t n);  // 1..6
uint8_t route_get();        // 0 = relays match no route

void ui_init();
void ui_poll();               // call from loop(): handles touch
void ui_draw_route();       // move the active-route marker
void ui_remote(bool on);   // true: show the remote screen and ignore buttons; false: back to local

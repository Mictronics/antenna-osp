#pragma once
#include <Arduino.h>

// Implemented in main.cpp, shared with the SCPI handler.
void route_set(uint8_t n);  // 1..6
uint8_t route_get();        // 0 = relays match no route

void ui_init();
void ui_poll();               // call from loop(): handles touch
void ui_draw_route();       // move the active-route marker
uint8_t theme_get();       // 0 normal, 1 high contrast, 2 light
void theme_set(uint8_t t);   // persisted in the settings file, redraws the screen
void ui_battery(uint8_t pct);  // 0..100, shown on the settings screen at 25 % steps
void ui_remote(bool on);   // true: show the remote screen and ignore buttons; false: back to local
uint32_t ui_last_touch();  // millis() of the last touch

#pragma once
#include <stdint.h>

// Provided by the application. Bit n = relay K(11+n) in state 1 (C-NO), n = 0..5.
uint8_t relays_get();
void relays_apply(uint8_t mask, uint8_t val);

// Remote lock of the local UI: set on every received line, cleared by SYST:LOC.
void remote_set(bool on);

// High-contrast theme (DISPlay:THEMe); implemented by the UI.
bool theme_get();
void theme_set(bool hc);

// Handle one complete line (no terminator); every reply, newline included, goes to out().
void scpi_handle(const char *line, void (*out)(const char *));

#pragma once
#include <stdint.h>

// Provided by the application. Bit n = relay K(11+n) in state 1 (C-NO), n = 0..5.
uint8_t relays_get();
void relays_apply(uint8_t mask, uint8_t val);

// Handle one complete line (no terminator); every reply, newline included, goes to out().
void scpi_handle(const char *line, void (*out)(const char *));

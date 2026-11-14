#pragma once
#include <stddef.h>
#include <stdint.h>

// Provided by the application. Bit n = relay K(11+n) in state 1 (C-NO), n = 0..5.
uint8_t relays_get();
void relays_apply(uint8_t mask, uint8_t val);

// Remote lock of the local UI: set on every received line, cleared by SYST:LOC.
void remote_set(bool on);

// High-contrast theme (DISPlay:THEMe); implemented by the UI.
uint8_t theme_get();  // 0 NORM, 1 HCON, 2 LIGHt
void theme_set(uint8_t t);

// Path storage on SD; implemented by the UI. paths_read() returns the bytes read, NUL-terminated, 0 if none.
void paths_write(const char *text);
size_t paths_read(char *buf, size_t cap);
void scpi_load_paths();  // replace the stored paths with the saved ones (also done before the first command)

// Handle one complete line (no terminator); every reply, newline included, goes to out().
void scpi_handle(const char *line, void (*out)(const char *));

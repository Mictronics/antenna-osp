# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

DIY clone of the Rohde&Schwarz OSP (switch/control platform) on the Waveshare RP2350-Touch-LCD-3.5 (RP2350B, 520 KB SRAM, no PSRAM, ST7796 480x320 landscape SPI display, FT6336 I2C touch).

## Build

PlatformIO, Arduino framework via the earlephilhower Arduino-Pico core on the community platform `maxgerhardt/platform-raspberrypi` (the official platform lacks RP2350).

- Build: `pio run`
- Upload: `pio run -t upload` (or hold BOOT and drag the UF2 onto the USB drive)
- Monitor: `pio device monitor`

## Hardware mapping

- RF switch tree, 5 relays K1..K5 on GPIO0,1,2,3,5 (header pins 6,8,10,12,16), active-low modules (`ACTIVE_LOW` in `src/main.cpp`). K1 picks the K2/K3 branch; K2 = IN1/IN2; K3 picks K4/K5; K4 = IN3/IN4; K5 = IN5/IN6. `ROUTE_MASK` in `src/main.cpp` maps route 1..6 to relay states (NC = first input of each pair; edit if wiring differs). The variant must define `PICO_RP2350B 1`, otherwise the core rejects GPIO >= 30 (SD CS is 31). GPIO4 is LCD MISO (SPI0), do not use.
- SCPI over RS485 via Waveshare TTL-to-RS485 (C) module on UART1 (`Serial2`): TX=GPIO8, RX=GPIO9, 115200 baud.
- Taken by board (from Waveshare examples in ~/git/RP2350-Touch-LCD-3.5): LCD SPI0 CLK18/MOSI19/MISO4/CS21/DC20/RST23/BL22; touch+IMU+RTC+codec on Wire1 SDA34/SCL35, touch RST24/INT25, IMU INT1 14, battery ADC 28. Camera uses GPIO0+ only if fitted. Free header GPIOs: 6,7,41-47.

## UI

Thin custom layer, no LVGL: `display` (ST7796 on SPI0, window blit/fill), `touch` (FT6336U on Wire1), `ui` (`SCREENS` table in `src/ui.cpp`: each screen = background `/name.rgb565` from SD + widgets, ROUTE selects input 1..6 and frames the active button, NAV switches screen; no image falls back to plain grey buttons). SD is on SPI1 (SCK26 MOSI27 MISO28 CS31), FAT32/exFAT. Images are 480x320 (landscape) big-endian RGB565, made with `tools/png2rgb565.py`. `route_set()`/`route_get()` in `main.cpp` are shared by SCPI and the UI. GPIO28 is also the battery ADC in Waveshare's examples (shared with SD MISO).

## SCPI

`src/scpi.cpp` implements an R&S OSP-style subset from the OSP user manual (`~/Downloads/OSP220-230-320_UserManual_en_17.pdf`, chapters 8-9, programming examples p.344-346). It emulates frame F01 with one OSP-B101 (6 SPDT) in slot M01: K11..K15 are the real relays, K16 is virtual. State 01 = C-NO = relay energised. Relay number `ee` accepts 01..06 and 11..16 (the manual is inconsistent). Implemented: common commands (`*IDN? *RST *CLS *OPC *ESR? *STB? ...`), `SYST:ERR[:NEXT]? / :ALL? / :PRES / :VERS?`, `ROUT:CLOS` (+`?`, `,VAL`, `,SUMM`, channel lists or path names), `ROUT:CLOS:COMB`, `ROUT:PATH[:DEF]` and `?`, `:DEL[:NAME]`, `:DEL:ALL`, `:CAT? [ACT]`, `:LAST?`, `:MULT`, `DIAG:SERV:HWIN?`. Not implemented: path export/import, `ROUT:PATH:COMB?`, `CONF:*`, trigger and legacy commands. Paths live in RAM only. Dispatch uses the Vrekrer SCPI parser (git dependency on the Mictronics fork `Mictronics/scpi-parser`, pinned by full commit SHA in `platformio.ini`; upstream 0.5.0 plus the opt-in `SCPI_QUOTE_AWARE`). It is header-only, so include it from `src/scpi.cpp` only. The library has no common commands and no error queue, so those and the status registers are our own. `-DSCPI_QUOTE_AWARE` (added in the fork) keeps `,`/`;` inside quotes and parentheses unsplit. Limits are raised by `-DSCPI_*` build flags in `platformio.ini` (the host test must use the same flags). Hooks `relays_get()/relays_apply()` are in `src/main.cpp`, which also splits RS485 input into lines. Replies end in `\n`.

ELEKTRA reaches a real OSP over VISA/LAN; here RS485 is fixed (EMC), so a TCP-to-RS485 middleware/proxy is needed on the PC side. `*IDN?` answer is `SCPI_IDN` in `src/scpi.cpp`; the HWIN names (OSP230, OSP-B101) are what module detection reads.

Host test of the parser (against the fetched library): `g++ -std=c++17 -Itools/host -Isrc "-I$(ls -d .pio/libdeps/*/*/src | head -1)" -DSCPI_ARRAY_SYZE=24 -DSCPI_MAX_TOKENS=40 -DSCPI_MAX_COMMANDS=48 -DSCPI_HASH_TYPE=uint16_t -DSCPI_QUOTE_AWARE tools/scpi_test.cpp src/scpi.cpp -o /tmp/scpi_test && /tmp/scpi_test` (`tools/host/Arduino.h` is a minimal Arduino stand-in).

Parser changes go in the fork (local repo `~/VisualStudioCode/scpi-parser`, master), not here. Push it, then pin the full 40-char SHA in `platformio.ini`; PlatformIO can't resolve short hashes or unpushed commits. Never edit `.pio/libdeps`.

## Open items

- Custom board in `boards/` + variant in `variants/` (RP2350B, 16 MB flash), pins from Waveshare examples. Wire0 pins in the variant are unused placeholders.
- RS485 direction control not implemented; confirm the module is auto-direction.
- Not yet tested on hardware or with ELEKTRA: *IDN? format, ROUT:PATH:LAST? quoting and module detection are guesses from the manual.
- Remote operation does not lock the touch UI (a real OSP does).
- Landscape: MADCTL 0x28 in `display.cpp` and the matching x/y swap in `touch.cpp` are untested; if the image is upside down use 0xE8 and flip the touch mapping, check hit rects on hardware.
- GPIO4 is claimed by SPI0 RX by default in the variant; the LCD is write-only, so it may be freed later.

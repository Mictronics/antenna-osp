# antenna-osp

Open Switching Platform (OSP): a DIY RF switch built on the
[Waveshare RP2350-Touch-LCD-3.5](https://www.waveshare.com/RP2350-Touch-LCD-3.5.htm).
It switches 6 RF inputs to 1 output with 5 relays, can be operated by touch, and
accepts SCPI commands over RS485.

> **Status:** the firmware builds and the SCPI parser passes a host-side test.

## Hardware

- Waveshare RP2350-Touch-LCD-3.5 (RP2350B, 16 MB flash, ST7796 480x320 landscape display, FT6336U touch)
- Waveshare TTL-to-RS485 (C) module on UART1: TX GPIO8, RX GPIO9, 115200 baud
- FAT32/exFAT SD card for the screen backgrounds
- 5V 8-Channel relay module with active-low signals on GPIO 0, 1, 2, 3, 5 (header pins 6, 8, 10, 12, 16)
- The 8-Channel relay module is driving five 12V RF relays RADIALL R570412000

The relays form a pyramid so every path has the same loss:

```
K1 ─┬─ K2 ─┬─ IN1
    │      └─ IN2
    └─ K3 ─┬─ K4 ─┬─ IN3
           │      └─ IN4
           └─ K5 ─┬─ IN5
                  └─ IN6
```

`ROUTE_MASK` in `src/main.cpp` maps each input to the relay states. It assumes that
the de-energised (NC) position selects the first input of each pair; edit the table if
your wiring differs.

## Build

PlatformIO with the Arduino-Pico core on the community platform
`maxgerhardt/platform-raspberrypi`:

```
pio run                 # build
pio run -t upload       # flash (or drag the UF2 onto the BOOT drive)
pio device monitor
```

## Touch UI

Thin custom layer without LVGL. Each screen is a full-size background image
`/name.rgb565` on the SD card (480x320, big-endian RGB565) plus a few widgets that
select a route or switch the screen. Without an image the buttons fall back to plain
grey. Convert artwork with `tools/png2rgb565.py`. The button rectangles are in the
`SCREENS` table in `src/ui.cpp`.

### Editing the artwork with Inkscape

The sources are `screens/svg/<name>-{dark,hc,light}.svg` (480x320 px pages). The SD
card wants `<name>.rgb565` for dark and `<name>-hc.rgb565` / `<name>-light.rgb565` for
the other themes (the splash files are `splash-*.rgb565`).

One-time setup: copy `tools/inkscape/rgb565_output.{inx,py}` into the Inkscape user
extensions folder (Edit > Preferences > System > "User extensions"; on Linux
`~/.config/inkscape/extensions/`), install Pillow for the Python that Inkscape uses
(`pip install pillow`), then restart Inkscape.

Per screen: edit the SVG, then File > Save a Copy, choose "Big-endian RGB565 480x320
(\*.rgb565)" and save as the name above into `screens/`. Copy the result to the SD card
root. Keep the page at 480x320 and the buttons where the `SCREENS` table in
`src/ui.cpp` expects them. Text on the settings screen uses a custom font, so the
version string is edited in the SVG by hand.

## Remote control

SCPI over RS485, one command per line. The device presents itself as frame F01 with one
6-way SPDT module in slot M01, where K11..K15 are real relays and K16 is virtual.
Implemented: the common commands (`*IDN? *RST *CLS *OPC *ESR? *STB?` and friends),
`SYST:ERR?`, `ROUT:CLOS`, `ROUT:PATH` (define, delete, catalog, last, multiple) and
`DIAG:SERV:HWIN?`. Not implemented: path export/import, `CONF:*`, trigger commands.

The device is RS485 only (to avoid Ethernet EMC noise). To control it from
VISA/LAN software, put a TCP-to-RS485 proxy on the PC side.

The SCPI dispatch uses a [fork of the Vrekrer SCPI parser](https://github.com/Mictronics/scpi-parser)
with the `SCPI_QUOTE_AWARE` option. To test the command handling without hardware, see
the host test command in `CLAUDE.md`.

## License

GNU General Public License v3.0, see [LICENSE](LICENSE). The Vrekrer SCPI
parser is MIT licensed.

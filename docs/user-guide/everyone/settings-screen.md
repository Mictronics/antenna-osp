# Settings screen

Set the screen brightness and read device information.

## Steps

1. On the main screen, touch **Settings**.
2. Touch one of the brightness buttons **25%**, **50%**, **75%** or **100%**. The screen changes at once and a green frame marks your choice.
3. Touch **Back** to return to the main screen.

> *Placeholder: photo of the Settings screen with one brightness button framed in green.*

## What you should see

- Under **BRIGHTNESS**, the chosen button is framed in green.
- An information panel shows **DEVICE** (OSP230 (clone)), **INTERFACE** (RS485 · 115200 8N1), **FIRMWARE** (the version, for example v0.1.0) and **FRAME / MODULE** (F01 · M01 OSP-B101).
- A battery symbol (empty, 25 %, 50 %, 75 % or full) is shown if a battery gauge is connected.
- A plug with a check mark replaces the battery symbol when the battery is fully charged and charging has stopped.

## Things to know

- Your brightness choice is saved on the SD card and used at the next start.
- The lowest level is 25 %. The screen is only fully dark through [automatic power saving](power-saving.md).
- The battery level is derived from the cell voltage using the pack's `OCV_SOC*` parameters (set in the pack, not in this firmware).
- The battery symbol is updated about once a minute and rounds to the nearest 25 %. Without a battery gauge it stays at full.
- The plug-with-check symbol appears when the battery level is 99 % or higher and almost no current flows (less than 30 mA). After charging stops it can take up to a minute to appear. It also appears on a full battery that is idle with almost no load, because the gauge cannot tell whether external power is connected.
- The colour theme cannot be changed here. It is set from a computer; see [SD card, themes and settings file](../advanced/sd-card-and-themes.md).

## Related

- [Automatic power saving](power-saving.md)

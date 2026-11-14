# Settings screen

Set the screen brightness and read device information.

## Steps

1. On the main screen, touch **Settings**.
2. Touch one of the brightness buttons **25%**, **50%**, **75%** or **100%**. The screen changes at once and a green frame marks your choice.
3. Touch **Back** to return to the main screen.

> 📸 *Placeholder: photo of the Settings screen with one brightness button framed in green.*

## What you should see

- Under **BRIGHTNESS**, the chosen button is framed in green.
- An information panel shows **DEVICE** (OSP230 (clone)), **INTERFACE** (RS485 · 115200 8N1), **FIRMWARE** (the version, for example v0.1.0) and **FRAME / MODULE** (F01 · M01 OSP-B101).
- A battery symbol (empty, 25 %, 50 %, 75 % or full) is shown if a battery gauge is connected.

## Things to know

- Your brightness choice is saved on the SD card and used at the next start.
- The lowest level is 25 %. The screen is only fully dark through [automatic power saving](power-saving.md).
- The battery symbol is updated about once a minute and rounds to the nearest 25 %. Without a battery gauge it stays at full.
- The colour theme cannot be changed here. It is set from a computer; see [SD card, themes and settings file](../advanced/sd-card-and-themes.md).

> ⚠️ *To confirm: the battery gauge (openUPS2) reading has not been tested on hardware yet.*

## Related

- [Automatic power saving](power-saving.md)

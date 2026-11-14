# Updating the firmware

## Before you start

- A USB cable to the board and a computer.
- Either PlatformIO to build it, or a ready-made `.uf2` file.

## Steps (UF2)

1. Hold the **BOOT** button on the board and connect USB. A drive appears.
2. Copy the `.uf2` file onto the drive.
3. The board restarts by itself.

## Steps (PlatformIO)

1. In the project folder run `pio run` to build, then `pio run -t upload` to flash.
2. The version shown on the Settings screen and in `*IDN?` is the new one.

## Things to know

- The SD card and your settings are not changed by an update.
- Paths defined over RS485 are kept on the SD card and survive an update.
- The on-screen version is part of the picture on the SD card. After a version change, restamp the pictures with `tools/set_version.sh` and copy them to the card; see the project README.

## Related

- [SD card, themes and settings file](sd-card-and-themes.md)

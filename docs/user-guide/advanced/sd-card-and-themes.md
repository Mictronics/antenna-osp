# SD card, themes and settings file

All screen pictures are on the SD card. Use a FAT32 or exFAT card.

## Files on the card (root folder)

| Files | Content |
|---|---|
| `main`, `settings`, `remote` | The three screens, normal (dark) theme, `.rgb565` |
| the same with `-hc` / `-light` | High contrast and light theme |
| `splash-dark`, `splash-hc`, `splash-light` | Start-up picture |
| `batt-0`, `batt-25`, `batt-50`, `batt-75`, `batt-100` (with `-hc` / `-light`) | Battery symbols on the Settings screen |
| `bright.txt` | Created by the device: brightness and theme |

Pictures are 480 x 320 pixels, big-endian RGB565, ready-made in the project's `screens` folder.

## Change the colour theme

1. Send `DISP:THEM NORM` (dark), `DISP:THEM HCON` (high contrast) or `DISP:THEM LIGH` (light).
2. The screen redraws and the choice is kept after a restart.

If a picture of the chosen theme is missing, that screen shows plain grey buttons.

## Reset the settings to defaults

1. Switch the device off and take out the SD card.
2. Delete `bright.txt` on a computer.
3. Put the card back in and switch on. Brightness is 100 % and the theme is normal.

## Things to know

- Without a card the device works with plain grey buttons.
- Edit the artwork with Inkscape as described in the project README.

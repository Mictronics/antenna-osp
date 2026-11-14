# Troubleshooting

| Symptom | Likely cause | What to do |
|---|---|---|
| Screen is dark | Backlight switched off after 5 minutes without a touch | Touch the screen once |
| Screen is dark right after power-on | No supply or supply too low | Check the supply |
| Buttons are plain grey with no labels | SD card missing, not FAT32/exFAT, or image files missing | Check the card and the files; see [SD card, themes and settings file](../advanced/sd-card-and-themes.md) |
| Everything is light or high contrast but you want dark | The colour theme is saved on the card | Set it from a computer (`DISP:THEM NORM`) or delete the settings file |
| Touching a button does nothing | The REMOTE screen is shown | Touch once to unlock |
| First touch does nothing | It only woke the screen | Touch again |
| Device switched back to IN 1 by itself | No touch and no route command for 1 hour | See [Automatic power saving](power-saving.md) |
| No green frame on any input | A computer set a relay combination that is not an input | Touch an input button |
| The computer gets no reply | Wiring, adapter or settings wrong | See [Connecting over RS485](../advanced/rs485-connection.md) |
| Battery symbol always full | No battery gauge answers | Expected without a gauge |
| Brightness is back at 100 % after restart | The card could not be written (write-protected or missing) | Check the SD card |

> ⚠️ *To confirm: troubleshooting for relay and wiring faults depends on your build and is not covered by the firmware.*

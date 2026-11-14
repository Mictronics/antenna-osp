# Remote control lock

When a computer talks to the device, the screen shows **REMOTE** with a padlock and the hint **Touch to unlock**.

## Why it appears

The screen changes to REMOTE as soon as the device receives any command line over RS485, even a query such as `*IDN?` or an invalid command. While it is shown, the input buttons are disabled so that nobody switches by touch during a measurement.

## Steps to return to touch control

1. Touch anywhere on the screen. The previous screen returns.

If the computer sends another command later, the REMOTE screen appears again. A computer can also return the screen with the command `SYST:LOC`.

## Things to know

- If the backlight is off, the first touch only switches it on. Touch again to unlock.
- Commands from a computer do not switch the backlight on. See [Automatic power saving](power-saving.md).

## Related

- [Command reference](../advanced/command-reference.md)

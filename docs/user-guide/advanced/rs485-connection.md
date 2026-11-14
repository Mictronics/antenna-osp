# Connecting over RS485

Control the device from a computer with text commands (SCPI).

## Before you start

- An RS485 adapter on your computer (USB to RS485).
- The device has a Waveshare TTL-to-RS485 (C) module fitted. It switches direction by itself, so no direction control is needed.

## Settings

| Setting | Value |
|---|---|
| Baud rate | 115200 |
| Data format | 8 data bits, no parity, 1 stop bit (8N1) |
| Line ending | Line feed (`\n`) or carriage return (`\r`) ends a command |
| Reply ending | Line feed (`\n`) |
| Longest command | 511 characters |

Send one command per line. Commands and keywords are not case sensitive, and long keywords can be shortened to the capital letters shown in the [command reference](command-reference.md).

## Steps

1. Connect the A and B lines of your adapter to the RS485 terminals of the device module.
2. Open the serial port with the settings above.
3. Send `*IDN?`. The reply is `DIY,OSP-EMU,000001,` followed by the firmware version.
4. The device shows the [REMOTE screen](../everyone/remote-lock.md).

## If something goes wrong

- No reply: swap A and B and check that both ends use 115200 baud.
- Still no reply: check the solder joints on the module and that its TX goes to the board's RX and the reverse.

## Things to know

- The device is RS485 only. To reach it from VISA/LAN software (for example ELEKTRA) you need a TCP to RS485 bridge on the PC side.
- Commands that return nothing (for example `ROUT:CLOS`) send no reply, so check with `SYST:ERR?`.

> ⚠️ *To confirm: the exact RS485 terminal labels and which board pins the module is wired to (GPIO8 and GPIO9 are used by the firmware).*

## Related

- [Command reference](command-reference.md)

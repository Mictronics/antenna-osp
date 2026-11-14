# Using the device from ELEKTRA

ELEKTRA controls the device through its **Generic Switching Unit** driver. This driver sends free text commands over a serial port, one per switching request. It needs no device identification and no network bridge: connect the USB RS485 adapter to the computer and select its port.

## Before you start

- The RS485 link works; see [Connecting over RS485](rs485-connection.md).
- ELEKTRA is installed with the Generic Switching Unit driver.

## Settings in ELEKTRA

| Setting | Value |
|---|---|
| Interface | The serial port of the USB RS485 adapter |
| Baud rate | 115200, 8 data bits, no parity, 1 stop bit |
| Line ending | `\n` |
| Reply | None expected; switching commands send no reply |

## Command for each input

Enter one command per input. Each command sets all five relays, so it works from any previous state and needs no `*RST` before it.

| Input | Command |
|---|---|
| IN 1 | `ROUT:CLOS (@F01M01(0011,0012,0013,0014,0015))` |
| IN 2 | `ROUT:CLOS (@F01M01(0011,0112,0013,0014,0015))` |
| IN 3 | `ROUT:CLOS (@F01M01(0111,0012,0013,0014,0015))` |
| IN 4 | `ROUT:CLOS (@F01M01(0111,0012,0013,0114,0015))` |
| IN 5 | `ROUT:CLOS (@F01M01(0111,0012,0113,0014,0015))` |
| IN 6 | `ROUT:CLOS (@F01M01(0111,0012,0113,0014,0115))` |

## What you should see

The device shows the [REMOTE screen](../everyone/remote-lock.md). After a command the relays click. Nobody can switch by touch until the screen is touched, and a touch returns it to local control.

## Things to know

- **Keep the 1 hour timer in mind.** If ELEKTRA does not send a switching command for an hour and nobody touches the screen, the device falls back to IN 1. Long measurements on one input need a repeated command. See [Automatic power saving](../everyone/power-saving.md).
- The backlight goes off after 5 minutes without a touch, even while ELEKTRA is controlling the device.
- To check a switch from the computer, send `ROUT:CLOS? (@F01M01(0111,0012,0013,0014,0015)),SUMM`. It replies `1` if the relays match.

## Related

- [Command reference](command-reference.md)

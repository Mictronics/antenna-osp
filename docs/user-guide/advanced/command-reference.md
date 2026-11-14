# Command reference

The device understands a subset of the Rohde & Schwarz OSP SCPI commands. It behaves as **frame F01** with one **OSP-B101** (6 changeover relays) in **slot M01**.

## Relays and inputs

Relays are called K11 to K15 (real) and K16 (virtual: the device remembers its state but nothing is switched). You can write them as 11 to 16 or as 01 to 06.

State **01** energises the relay, **00** releases it. An item in a list is the state followed by the relay: `0112` means "relay K12 energised".

| Input | Relays energised |
|---|---|
| IN 1 | none |
| IN 2 | K12 |
| IN 3 | K11 |
| IN 4 | K11, K14 |
| IN 5 | K11, K13 |
| IN 6 | K11, K13, K15 |

## Switching

| Command | Effect |
|---|---|
| `ROUT:CLOS (@F01M01(0111,0113,0115))` | Energises K11, K13, K15 (IN 6). Only the listed relays change. |
| `ROUT:CLOS (@F01M01(0111,0012,0013,0014,0015))` | IN 3: K11 on, the others off |
| `*RST` or `SYST:PRES` | All relays off (IN 1) |
| `ROUT:CLOS "name"` | Applies a stored path |
| `ROUT:CLOS:COMB "a","b"` | Applies several stored paths together |

`ROUT:CLOS` adds to the current state. Send `*RST` first if you want a clean start.

## Queries

| Command | Reply |
|---|---|
| `ROUT:CLOS? (@F01M01(0112,0114))` | `1,0`: 1 where the relay is in the requested state |
| `ROUT:CLOS? (@F01M01(0112)),VAL` | The real relay states |
| `ROUT:CLOS? (@F01M01(0112)),SUMM` | 1 if all listed relays match |

## Paths

A path is a stored relay setting with a name. Paths are kept in memory only and are lost at power-off. You can store up to 24, each with up to 16 relays and a name of up to 39 characters.

| Command | Effect |
|---|---|
| `ROUT:PATH:DEF "IN3",(@F01M01(0111,0012,0013,0014,0015))` | Defines a path |
| `ROUT:PATH:DEF? "IN3"` | Returns its definition |
| `ROUT:PATH:DEL "IN3"` | Deletes it |
| `ROUT:PATH:DEL:ALL` | Deletes all |
| `ROUT:PATH:CAT?` | Lists all names; `ROUT:PATH:CAT? ACT` lists those currently active |
| `ROUT:PATH:LAST?` | The last path or setting used |
| `ROUT:PATH:MULT ON\|OFF` | Accepted and reported back by `ROUT:PATH:MULT?` |

## Common and system commands

| Command | Effect |
|---|---|
| `*IDN?` | `DIY,OSP-EMU,000001,<version>` |
| `*RST` | Resets the relays |
| `*CLS` | Clears the error queue and status |
| `*OPC`, `*OPC?` | Operation complete (reply 1) |
| `*ESR?`, `*ESE`, `*STB?`, `*SRE` | Status registers |
| `SYST:ERR?` or `SYST:ERR:NEXT?` | Next error, or `0,"No error"` |
| `SYST:ERR:ALL?` | All queued errors |
| `SYST:VERS?` | `1999.0` |
| `SYST:LOC` | Returns the screen from REMOTE to the previous screen |
| `DIAG:SERV:HWIN?` | Frame, main board and module identification |
| `DISP:THEM NORM\|HCON\|LIGH` and `DISP:THEM?` | Colour theme; see [SD card, themes and settings file](sd-card-and-themes.md) |

Up to 8 errors are queued. Errors you may see: -102 Syntax error, -109 Missing parameter, -113 Undefined header, -200 Execution error (module not present, path not found), -222 Data out of range, -224 Illegal parameter value, -321 Out of memory.

## Things to know

- Every received line shows the [REMOTE screen](../everyone/remote-lock.md).
- A route change restarts the 1 hour relay timer, queries do not. See [Automatic power saving](../everyone/power-saving.md).
- Not implemented: path export and import, `ROUT:PATH:COMB?`, `CONF:*`, trigger and legacy commands.

## Related

- [Connecting over RS485](rs485-connection.md)
- [Using the device from ELEKTRA](elektra.md)

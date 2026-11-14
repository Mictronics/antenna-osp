# Automatic power saving

The device saves energy by itself when you forget to switch it off.

## What happens

| After | Without | The device |
|---|---|---|
| 5 minutes | a touch | Switches the screen backlight off. |
| 1 hour | a touch **and** a route change from the computer | Switches all relays off, which means input **IN 1**. |

## Steps to wake the screen

1. Touch the screen anywhere. The backlight comes back at your saved brightness.

The touch that wakes the screen does nothing else, so you cannot press a button by accident.

## Things to know

- Commands from a computer do **not** switch the backlight on and do not restart the 5 minute timer. During remote operation the screen goes dark after 5 minutes of no touch.
- Any route change from the computer restarts the 1 hour timer, and so does any touch. Queries do not.
- **Long measurements:** if you hold one input for more than an hour without touching the screen or sending a route command, the device drops back to IN 1. Send a route command from time to time, or touch the screen.
- The times are fixed in the firmware and cannot be changed from the device.
- The 5 minute and 1 hour timers start again after every power-on.

> ⚠️ *To confirm: a separate shutdown after about 8 hours is planned in the battery pack (openUPS2). It is not part of this firmware, so it is not described here.*

## Related

- [Remote control lock](remote-lock.md)

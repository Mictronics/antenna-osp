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

## Battery pack shutdown

If the device runs from the battery pack (openUPS2), the pack itself switches the device off after 8 hours on battery. This is set in the pack, not in this firmware, so the device cannot change or show it. Supply the device from the external power again to restart it.

**Charging:** the pack charges only from an input voltage of 9 to 28 V, with a charging current of 2 A.

## Related

- [Remote control lock](remote-lock.md)

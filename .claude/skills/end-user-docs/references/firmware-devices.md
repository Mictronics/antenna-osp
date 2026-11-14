# Discovery notes: firmware, devices and hardware-bound software

Read this when the code runs on a device (microcontroller, SBC, appliance) — the user's experience is buttons, LEDs, a small display, a serial console, radio/network links, and a web or phone UI.

The goal: reconstruct the *user-visible behavior* of the device from code that was never written as a spec. Behavior lives in input handling, state machines, UI tables and the settings store; wiring and appearance live outside the code.

## Table of contents

1. Orient: framework, target, entry points
2. Inputs: buttons, encoders, touch
3. Outputs: LEDs, buzzer, display
4. Menus and screens
5. Serial, USB, network and radio interfaces
6. Settings and persistence
7. Lifecycle: boot, setup, update, reset, power
8. Errors and troubleshooting material
9. What the code cannot tell you

## 1. Orient

- Identify framework and target from the build files: `platformio.ini` (env names = supported boards), `CMakeLists.txt`/`sdkconfig`/`Kconfig` (ESP-IDF, Zephyr), `.ino` (Arduino), `Cargo.toml` + HAL crate, `Makefile` (AVR/STM32), QMK/ZMK keymaps, Linux userspace services with systemd units.
- Multiple boards/variants: note per-variant differences (a button that exists on one board, a display on another). Document the common behavior once and call out variants in a table, rather than writing one guide per board.
- Find the entry point (`setup()`/`loop()`, `app_main`, `main`) and what starts at boot. You are looking for *features*, not for reading every driver.
- Read README, wiki pages, `docs/`, release notes and CLI/help strings first: they contain the author's own words for features.

## 2. Inputs

Search for GPIO/input handling: `digitalRead`, `attachInterrupt`, `gpio_`, `OneButton`, `Bounce2`, encoder libraries, touch APIs, key matrices.

For each input capture: its name/label as the user would know it (use the board silkscreen/README name if given, otherwise "Button 1"), each gesture (short, long, double, combination, hold-at-boot), the thresholds (take the actual constants for debounce, long-press and timeout durations), and the effect — which may depend on the current screen or mode. Build a gesture × context table. Boot-time combinations (hold X while powering on → recovery/bootloader/factory reset) are especially valuable for users.

## 3. Outputs

- **LEDs:** find the pattern definitions (colors, blink periods, sequences) and map each to the state that triggers it. Describe in words a human can observe ("blinks twice every few seconds"), not in milliseconds, unless timing matters.
- **Buzzer/haptics:** same approach — which events beep, how many times.
- **Display:** collect the literal strings shown (status texts, error messages, units) and what each screen conveys. Mention icons by what they look like/mean.
- **Other outputs** the user observes: relays, motors, outgoing signals — what the user sees happen, and warnings if the action has safety implications that the code or README states.

## 4. Menus and screens

Menu systems are usually tables or screen/page enums. Reproduce the menu tree as the user navigates it (Main → Settings → Display → Brightness), with the labels exactly as shown, and note how to move, select, go back and exit. A menu tree diagram (text or list) is a high-value part of a device guide.

## 5. Serial, USB, network and radio interfaces

- **Console / AT / CLI commands:** from the parser, list each command, arguments, example, output. These are user-facing for advanced users; put them in an "advanced" reference. Include the serial settings (baud rate, line ending) found in the code.
- **Web UI / REST served by the device:** how to reach it (access-point name and default address, mDNS name, port) as the code defines them, then document screens using `web-apps.md`.
- **BLE/Wi-Fi/MQTT/LoRa/etc.:** what the user must do to connect or pair, what names/topics/channels they'll see, what defaults apply (credentials and keys: document that defaults exist and must be changed; never paste real secrets found in the repo).
- **Companion apps / host tools:** document their user-visible workflow the same way as any app.

## 6. Settings and persistence

Find the settings store (`Preferences`/NVS, EEPROM, config file, protobuf config, Kconfig options exposed to users) and the code that validates it. For each setting: user-facing name, what it does, allowed range/values, default, where it can be changed (screen, web, serial, config file), whether a reboot is needed, whether it survives power loss and firmware updates. Distinguish *runtime settings* (users change them) from *build-time options* (only people compiling the firmware — put these in an advanced "building from source" article).

## 7. Lifecycle

- **First boot / provisioning:** what the device does with no config (setup AP, pairing mode, default state) and the steps to get it configured.
- **Normal operation:** what a healthy device shows; modes (sleep, low power, transmit) and how users enter/leave them.
- **Updating firmware:** every supported route (OTA via web or app, USB/DFU/UF2, esptool/PlatformIO upload, bootloader button combos). Document the exact tool and command from the project's scripts/README, what is preserved, and the warning not to remove power mid-update if the project says so.
- **Factory reset / recovery:** gestures or commands, and what is erased.
- **Power:** battery/charging behavior and indicators if the code handles them; shutdown procedure.

## 8. Errors and troubleshooting material

Grep for error strings, `LOG_E`/`ESP_LOGE`/`Serial.println("ERR…")`, fault LED patterns, watchdog/brown-out handling, retry loops and "not found" paths (sensor missing, SD card absent, radio init failed). Each one is a troubleshooting row: what the user observes → what it means → what to try. Keep to causes the code supports; where you suspect hardware causes (loose wiring, weak supply), say so plainly only if the code's messages point there.

## 9. What the code cannot tell you

These go to the open-questions list and appear as visible placeholders, never as guesses:

- Physical layout: where buttons/LEDs/ports sit, what the case looks like, orientation.
- Wiring and pin-to-part mapping for DIY builds, unless the README or pin definitions give it (if they do, a "Connections" table is user-facing and worth including for builders).
- Supported hardware revisions not declared in the build files.
- Whether a half-implemented feature is meant to be used.
- Anything about legal/RF compliance, safety ratings, battery life or range.

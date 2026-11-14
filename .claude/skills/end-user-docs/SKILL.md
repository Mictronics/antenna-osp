---
name: end-user-docs
description: Generate and keep up to date end-user documentation (Markdown) for an existing codebase — plain-language, task-oriented help for the people who USE the product, not the engineers who build it. Works for web apps (Vue/React/etc.), CLIs, desktop/mobile apps, and especially firmware that runs on a device or hardware (buttons, LEDs, display menus, serial console, web UI on the device, config, flashing). Use this whenever the user asks for a user guide, user manual, bedienungsanleitung, handbuch, help docs, quick-start, onboarding docs, "how do I use this", getting-started or troubleshooting docs for code/firmware/a device, or wants existing user docs refreshed after code changes — even if they only say "document how to use this project" or "write a manual for this firmware". If the request is about documenting the code structure, components or screens for engineers, this is not the right skill.
---

# End-User Documentation from Code

## Purpose and audience

Turn an existing codebase into documentation for the people who *use* what it builds. The reader has never seen the code and never will. For a web app that is a person at a browser; for firmware it is a person holding a device who may not even have a screen to read — they have a few buttons, some LEDs, maybe a serial port. They want to know: what can this do, how do I do it, what does it look like when it works, and what do I do when it doesn't.

The code is your research material, not your subject. Read it to learn what the product does, then write it down in terms of what the user sees, presses, types and observes. Two things follow from that:

- **Translate, don't transcribe.** Say "hold the left button for 3 seconds to enter setup mode" — not "`handleButton()` calls `enterSetup()` when `held_ms > 3000`". Say "filter the list by status" — not "the DataTable's filter prop".
- **Don't invent.** Code tells you behavior; it rarely tells you what the case looks like, where the buttons physically are, or which pin is wired to what on the user's board. When the code doesn't establish a fact, write a clearly marked placeholder or add it to the open-questions list (Step 5) instead of guessing. A confidently wrong manual is worse than one with visible gaps, because users follow it literally — and on hardware a wrong wiring or flashing instruction can damage something.

If you catch yourself writing a file path, function name, class, store, prop or register in running text, stop and rewrite it as what the user does or observes. The exception is anything the user must *type or select themselves* (a CLI command, a config key, a serial command, a pin label on their board) — those are user-facing and belong in the docs, in code formatting.

## Step 1: Identify the product and its "surfaces"

First work out what kind of thing this is and every place a user touches it. A product often has several surfaces; each needs coverage.

| Surface | Typical places to look in the code |
|---|---|
| Web UI (in a browser, or served by the device itself) | router, views/pages, templates, static HTML in firmware |
| Command line | argument parsers, `--help` text, subcommand definitions, README usage |
| Physical controls | GPIO/input handlers: buttons (short/long/double press), encoders, switches, touch, key matrices |
| Indicators | LED, buzzer/beeper, display code, boot/status patterns |
| On-device display menus | menu tables, screen/page definitions, UI libraries (LVGL, u8g2, etc.) |
| Serial / USB / network console | command parsers, AT commands, MQTT topics, REST endpoints, BLE characteristics |
| Configuration | config files, settings/NVS/EEPROM keys, Kconfig, `config.h`, environment variables, build flags |
| Companion app / host tool | flasher scripts, provisioning tools, mobile/desktop clients |
| Lifecycle | install/build/flash procedure, first boot, factory reset, updates (OTA/DFU), shutdown/power |

Find the project root and build system (`package.json`, `platformio.ini`, `CMakeLists.txt`, `Makefile`, `Cargo.toml`, `pyproject.toml`, …), read the README and any existing docs, and identify which surfaces exist. Then read the matching reference file for the discovery details:

- Web apps (Vue and similar): `references/web-apps.md`
- Firmware, devices and hardware-bound software: `references/firmware-devices.md`

Read both if the project has both (e.g. a device that serves a web UI).

## Step 2: Derive what users can do, and for whom

While reading, capture for each capability:

1. **What it is for** — the goal in the user's words. Infer it from labels, menu text, log messages, help strings, README prose and comments rather than from identifiers.
2. **How the user triggers it** — the exact button/gesture, menu path, command, URL or setting, using the literal visible labels.
3. **What they observe** — display text, LED pattern, beep, response, output.
4. **Who may do it** — everyone, or gated (admin role, a hidden/long-press mode, a build-time flag, a "maintainer" command). This decides which section the topic goes in.
5. **Side effects and limits** — things that erase data, reboot the device, transmit radio/network traffic, take a long time, or have valid ranges.

Skip anything that is not user-facing: internal tasks and queues, debug-only output, test code, driver internals, intermediate redirects, loading spinners. If a debug feature is something a power user would plausibly use (verbose serial logging, diagnostic screen), put it in an advanced section rather than dropping it.

## Step 3: Organize by audience, then by task

Create `docs/user-guide/` (or follow the project's existing docs convention if there is one) with:

- **For everyone** — what regular users need.
- **For advanced users / admins** — gated features, configuration deep-dives, flashing/building, diagnostics.

Inside each section group by *task or feature area*, never by source file, route or module. People look for "How do I pair the device?", not "The BLE module". When several code paths add up to one task (a setup wizard, a multi-step provisioning flow), write one article for the whole flow.

For **devices and firmware**, make sure the guide also contains these, since a user cannot get started without them (derive the content from the code; mark gaps):

- **Getting started** — what you need, first power-on, first-time setup, what a healthy start looks like.
- **Controls and indicators reference** — tables mapping each button gesture, switch and display screen to its effect, and each LED/buzzer pattern to its meaning.
- **Settings reference** — every user-changeable setting: where to change it, what it does, allowed values, default, whether it needs a reboot.
- **Updating the firmware** and **resetting to factory defaults** — the procedure, what is preserved, what is erased.
- **Troubleshooting** — symptom → likely cause → what to do, built from the error states, LED/log messages and failure paths in the code.
- **Safety and care notes** — only what the code or docs support (battery handling, RF/transmit warnings, power limits). Do not invent regulatory or safety claims.

## Step 4: Write the articles

Use plain, second-person, instructional language throughout ("you", "press", "hold", "select", "type"). Match the language of the project's UI/existing docs, or the language the user wrote their request in; if unsure, ask once.

### `index.md`

A short welcome paragraph (what this product lets you do, written for someone meeting it for the first time, including what hardware or accounts they need), then a table of contents split by audience with a one-line description of what each article lets the reader accomplish.

### Task articles, e.g. `everyone/pairing-with-your-phone.md`

```markdown
# <Task phrased as what the user wants to do>

<One or two sentences: what this lets you do and why you'd want to.>

## Before you start

- <Prerequisites: hardware, firmware version, account, cable, app>

## Steps

1. <Concrete step using the literal label/gesture: "Press and hold **Button A** for 3 seconds">
2. <What you should now see/hear: "The display shows **Pairing…** and the LED blinks blue">
3. <...>

> 📸 *Placeholder: <exactly what to capture — e.g. "photo of the device with the pairing LED lit", "the Settings screen with the Language row highlighted">*

## What you should see

<The normal outcome, so the reader can tell it worked.>

## Things to know

- <Limits, valid ranges, data loss, reboots, time taken, who is allowed to do this>

## If something goes wrong

- <Symptom> → <cause and fix>

## Related

- <Links to related articles, including the admin/advanced counterpart>
```

Drop sections that don't apply (a simple task needs no "Before you start"); an empty heading is noise.

### Reference tables (devices)

Tables beat prose for lookups. Example:

```markdown
| Gesture | Where | What happens |
|---|---|---|
| Short press **A** | Any screen | Next page |
| Hold **A** 3 s | Home screen | Opens the setup menu |
```

```markdown
| LED | Meaning |
|---|---|
| Green, steady | Connected and ready |
| Red, blinking fast | Battery low |
```

Only list a row if you found it in the code. Timings (e.g. long-press duration) must come from the code's constants — read them, don't round them from memory.

### Placeholders for visuals

You cannot take photos or screenshots. Wherever a visual would materially help (a device's front panel, a first-run screen, a wiring step), insert the callout above — never an actual image link — and describe precisely what to capture, so whoever adds it doesn't have to guess. A labelled diagram of the controls is worth a placeholder in any device guide.

## Step 5: Verify, mark uncertainty, cross-link

1. **Trace every instruction back to the code.** Before finalizing, re-check each numbered step, table row and default value against the source. This is the part that makes the guide trustworthy.
2. **Keep an open-questions list.** Anything the code cannot answer (physical layout, how a part is wired, intended behavior when code and comments disagree, whether a feature is finished or experimental) goes into `docs/user-guide/_open-questions.md` as a checklist, each item naming the article it affects. In the article itself, mark the spot with a visible `> ⚠️ *To confirm: …*` callout. Keep these few and specific; their job is to be resolved, not to hedge the whole guide.
3. **Flag work-in-progress features.** If code is behind a disabled flag, `TODO`, or `#if 0`, either leave it out or label it clearly as experimental — don't document it as working.
4. **Cross-link, don't duplicate.** Where a feature has an everyone-side and an admin/advanced side (a user changes their own name; an admin renames anyone; a user changes a setting on the device screen; an advanced user sets it over serial), document each in its own section and link both ways under "Related".

## Step 6: Updating existing docs

If the user-guide folder already exists, this is a refresh:

1. Read the existing articles first. Preserve anything deliberately written for the reader (tips, caveats, real screenshots/photos, hand-tuned wording).
2. Re-derive what changed from the current code: new capabilities get new articles; removed ones get flagged rather than silently deleted (they may have moved); changed flows get their steps and tables updated.
3. Never replace an existing screenshot or photo with a placeholder; add placeholders only for new content that has none.
4. Summarize what changed in your reply — new, updated, flagged-as-removed, and open questions — so the user can review before committing.

## What NOT to include

- No file paths, function/class/component names, store or prop names, register addresses, or task/thread names in running text.
- No library jargon ("Dialog", "DataTable", "RTOS task", "ISR") — describe what the user experiences ("a pop-up asking you to confirm", "the device restarts").
- No implementation caveats ("this triggers an API call", "this writes to NVS") — say what matters in user terms ("your settings are kept after a restart", "this erases all saved pairings").
- No guesses about hardware you can't see in the code, and no regulatory/safety/performance claims the project doesn't itself make.

## Finishing

Report briefly: which surfaces you found, what articles you created or updated, and the open questions that need the user's answer. Mention where the screenshot/photo placeholders are so they know what to supply.

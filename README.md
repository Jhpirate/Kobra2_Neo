# Kobra2_Neo

Custom Marlin firmware for the **Anycubic Kobra 2 Neo**, built from Anycubic's released source. Prints sent from OctoPrint show on the printer's screen the same way as SD card prints, and USB printing is smoother.

> **Use at your own risk.** Keep a copy of the stock firmware so you can flash back.

| | |
|---|---|
| Mainboard | Anycubic Trigorilla `AC_TRI_F1`, STM32F103RET6 (512 KB flash, 64 KB RAM) |
| Display | 320×240 TFT with rotary knob |
| Firmware start | `0x08009000` (bootloader offset `0x9000`) |
| Marlin base | `bugfix-2.1.x` (Anycubic fork) |
| Version shown on the printer | `V1.6.0.0`, "AC Kobra 2 Neo (OctoPrint UI)" |

## Changes from stock

### New features

- **OctoPrint printing screen.** `M75` opens the same print screen an SD print uses, with the elapsed time, a progress bar, and pause and stop buttons. The title is the file name from `M75 <name>` (without `.gcode`), or "OctoPrint" if no name is given. `M77` shows "Print finished". `M77 C` (cancel) goes back to the home screen.
- **Pause, resume and stop from the printer during OctoPrint prints.**
  - **Pause** parks the head and tells OctoPrint the print is paused (`//action:paused`).
  - **Resume** after a pause started in OctoPrint asks OctoPrint to resume (`//action:resume`).
  - **Stop** tells OctoPrint to cancel (`//action:cancel`), clears queued commands, stops the motors and turns off the heaters.
- **Progress bar from `M73 P<percent>`.** The percentage can come from the slicer or an OctoPrint plugin.
- **Settings menu while printing.** Stock firmware locks the Settings button during a print. Now it opens, with the items that would move or reset the printer hidden: Move axis, Disable steppers, Filament change, Bed leveling, Calibration and Restore defaults.
- **Cancel single objects (`M486`).** Skip a failed part and keep printing the rest. Use OctoPrint or a slicer that labels objects, such as OrcaSlicer. There's no menu for this on the printer.
- **Optional filament purge before nozzle (Z offset) calibration.** Off by default. Set `CALIBRATION_FILAMENT_PURGE` to `1` to turn it on.

### Adjusted behavior

- **USB/serial**
  - MeatPack compression on serial port 1
  - Command buffer raised from 4 to 8, and a 4-byte TX buffer
  - Planner buffer raised from 16 to 32 moves, so OctoPrint can stream many short segments without pauses
  - Emergency parser on: `M108` (cancel heat-up wait), `M112` and `M410` act immediately instead of waiting in the queue
- **Host communication**
  - Host action commands and prompts on
  - `M575` changes the baud rate without reflashing (default `115200`)
  - The printer tells OctoPrint when it pauses or resumes on its own (LCD pause, `M600`)
- **Heater icons follow temperature.** Blue when cold, yellow once 40% of the way to the target, orange-red within the last 10%. A heater that's switched off shows yellow while it's still above 50 °C. The speed and Z offset icons also have their own colors.
- **`M117` restarts the print timer only once per print.** Anycubic's slicer output uses `M117` to mark when the print actually starts (after heating). Stock firmware restarted the timer on every `M117`, even when idle. That made the printer look busy, and it reset the timer whenever an OctoPrint plugin sent per-layer status messages.
- **`M73 R` (remaining time) turned off.** The screen has no space to show it.

### Main files edited

| File | Change |
|---|---|
| [`Marlin/src/lcd/menu/menu.h`](Marlin/src/lcd/menu/menu.h) | **Version number and device name** (`FIRMWARE_VER`, `DEVICE_NAME`) shown in the printer's info screen |
| [`Marlin/Configuration.h`](Marlin/Configuration.h) | `M575` baud rate change |
| [`Marlin/Configuration_adv.h`](Marlin/Configuration_adv.h) | Buffers, MeatPack, host actions and prompts, `M73`, emergency parser, `M486` |
| [`Marlin/src/gcode/parser.cpp`](Marlin/src/gcode/parser.cpp) | Text after `M75` read as one name, so names with spaces or capital letters come through whole |
| [`Marlin/src/lcd/marlinui.cpp`](Marlin/src/lcd/marlinui.cpp) / [`.h`](Marlin/src/lcd/marlinui.h) | Host print start and end, pause, resume and stop buttons |
| [`Marlin/src/MarlinCore.cpp`](Marlin/src/MarlinCore.cpp) | Cleanup when an OctoPrint print is stopped from the printer |
| [`Marlin/src/gcode/stats/M75-M78.cpp`](Marlin/src/gcode/stats/M75-M78.cpp) | `M75` / `M77` / `M77 C` connected to the print screen |
| [`Marlin/src/gcode/lcd/M117.cpp`](Marlin/src/gcode/lcd/M117.cpp) | Timer only restarts during a print |
| [`Marlin/src/lcd/tft/ui_320x240.cpp`](Marlin/src/lcd/tft/ui_320x240.cpp) | Status screen: OctoPrint title, icon colors |
| [`Marlin/src/lcd/menu/menu_main.cpp`](Marlin/src/lcd/menu/menu_main.cpp), [`menu_configuration.cpp`](Marlin/src/lcd/menu/menu_configuration.cpp) | Menu items hidden while printing |
| [`Marlin/src/HAL/STM32/autoGetZoffset.h`](Marlin/src/HAL/STM32/autoGetZoffset.h) / [`.cpp`](Marlin/src/HAL/STM32/autoGetZoffset.cpp) | `CALIBRATION_FILAMENT_PURGE` option (`0` = off, `1` = on) |
| [`buildroot/share/PlatformIO/boards/ac_tri_f103re.json`](buildroot/share/PlatformIO/boards/ac_tri_f103re.json) | Board definition, so it doesn't have to be installed in PlatformIO |
| [`buildroot/share/PlatformIO/scripts/marlin.py`](buildroot/share/PlatformIO/scripts/marlin.py) | Fix for newer Python versions |

## Building

### 1. Install Anycubic's STM32 framework (required)

The build needs **Anycubic's modified `framework-arduinoststm32` 4.10900.200819**, not the stock one PlatformIO downloads. The stock one compiles without errors, but it lacks the printer's board variant (pin map, linker script at `0x9000`) and Anycubic's driver changes (ADC with DMA for temperatures, flash write timing, SD card clock and timeouts).

Replace PlatformIO's package folder with Anycubic's:

- Linux: `~/.platformio/packages/framework-arduinoststm32`
- Windows: `C:\Users\<you>\.platformio\packages\framework-arduinoststm32`

The correct one has a `variants/AC_TRI_F103RE` folder, and its `README.md` ends with `framework-arduinoststm32_kobra2_neo`.

### 2. Build

**VS Code:** install the **PlatformIO IDE** and **Auto Build Marlin** extensions (VS Code suggests both when you open the folder). Open the repository root, open Auto Build Marlin from the activity bar, and click **Build** for `ac_tri_f1`.

**Command line:**

```sh
pio run -e ac_tri_f1
```

Output: `.pio/build/ac_tri_f1/firmware.bin`

## Flashing

1. Copy `firmware.bin` to the root of a FAT32 microSD card. Leave no other `.bin` files in the root (rename old ones to `.bin.old`).
2. Insert the card and power-cycle the printer. It flashes during startup.
3. Check your settings afterwards, and run leveling and Z offset calibration again if anything looks off.

## Linux notes

**Mount the card with `shortname=winnt`, or the printer won't flash.** By default Linux saves `firmware.bin` as `FIRMWARE.BIN` plus a long-name entry. The bootloader only finds the single lowercase short name that Windows writes, so it ignores the file. The file is still there and correct, but nothing flashes.

To make it permanent, create `/etc/udisks2/mount_options.conf`:

```ini
[defaults]
vfat_defaults=uid=$UID,gid=$GID,shortname=winnt,utf8=1,showexec,flush
```

This only affects FAT drives mounted by the desktop (USB sticks and SD cards). It doesn't affect NTFS drives or anything mounted from `/etc/fstab`. To limit it to one card, use `[/dev/disk/by-uuid/XXXX-XXXX]` instead of `[defaults]` (find the UUID with `lsblk -o NAME,UUID`).

To do it for a single mount instead:

```sh
udisksctl mount -b /dev/sdX1 -o shortname=winnt
```

Also on Linux:
- **Delete the old `firmware.bin` before copying a new one.** Overwriting it keeps the old directory entry.
- **Eject the card before pulling it** (or run `sync`), so the file is fully written.
- **The project path can contain spaces.** The build works with them as long as the Anycubic framework is installed in `~/.platformio`.

## OctoPrint setup

Slicers usually don't send `M75`/`M77`, so add them in **OctoPrint → Settings → GCODE Scripts**:

| Script | Content |
|---|---|
| Before print job starts | `M75 {{ event.name }}` |
| After print job completes | `M77` |
| After print job is cancelled | `M77 C` |
| After print job is paused | `M76` |
| Before print job is resumed | `M75` |

`{{ event.name }}` puts the file name on the print screen. If OctoPrint leaves it empty, the screen shows "OctoPrint".

**Progress bar:** turn on `M73` output in your slicer's printer profile, or use an OctoPrint plugin that sends `M73` (for example *M73 Progress*). Use one or the other, not both. Without `M73` the bar stays empty.

**Connection:** `115200` baud. To use another speed, send `M575 B<rate>` and reconnect at that rate.

## Credits

- [Marlin Firmware](https://github.com/MarlinFirmware/Marlin)
- Anycubic, for the Kobra 2 Neo source release
- [that-ambuj/Kobra2_Neo](https://github.com/that-ambuj/Kobra2_Neo), for the host action commands, MeatPack, buffer and `M575` changes

## License

GPL-3.0, like Marlin. See [LICENSE](LICENSE).

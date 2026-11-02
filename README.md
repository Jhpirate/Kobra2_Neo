# Kobra2_Neo

Custom Marlin firmware for the **Anycubic Kobra 2 Neo**, built from Anycubic's released source. It adds better OctoPrint (USB host) support and shows host prints on the printer's screen the same way as SD card prints.

> **Use at your own risk.** Flashing custom firmware can leave your printer in a state you have to recover from. Keep a copy of the stock firmware on hand before flashing.

## Hardware

| | |
|---|---|
| Printer | Anycubic Kobra 2 Neo |
| Mainboard | Anycubic Trigorilla `AC_TRI_F1` |
| MCU | STM32F103RET6 (72 MHz, 512 KB flash, 64 KB RAM) |
| Display | 320×240 TFT with rotary knob |
| Bootloader offset | `0x9000` |
| Marlin base | `bugfix-2.1.x` (Anycubic fork) |

## Changes from stock

### OctoPrint and USB host printing
- **Host prints show the SD-style printing screen.** `M75` switches the screen to the print status view, with "OctoPrint" as the title, the elapsed time, a progress bar, and pause/stop buttons. `M77` shows the "Print finished" screen. `M77 C` goes back to the home screen after a cancel.
- **Pause and stop work from the printer.** The pause button asks OctoPrint to pause. The stop button sends `//action:cancel` so OctoPrint stops streaming.
- **Progress bar from `M73`.** `SET_PROGRESS_MANUALLY` is on, so `M73 P<percent>` sent by the slicer or an OctoPrint plugin fills the bar.
- **Menus hidden during a print.** Menus that are unsafe during a print are hidden for host prints, the same as for SD prints.
- **Host action commands and prompts** (`HOST_ACTION_COMMANDS`, `HOST_PROMPT_SUPPORT`).
- **Less stuttering over USB.** MeatPack compression is on for serial port 1, the command buffer is larger (`BUFSIZE 8`), there is a TX buffer (`TX_BUFFER_SIZE 4`), and serial overrun protection is on.
- **`M575` baud rate change.** You can change the serial baud rate (default `115200`) without reflashing.

### Screen
- **Heater icons change color with temperature.** Blue when cold, yellow once 40% of the way to the target, orange-red within the last 10%. An icon for a heater that is off stays yellow while it is still above 50 °C.
- Speed and Z offset tiles have their own icon colors.

### Calibration
- **Optional filament purge during nozzle calibration.** Set `CALIBRATION_FILAMENT_PURGE` in [`autoGetZoffset.h`](Marlin/src/HAL/STM32/autoGetZoffset.h) to `1` to load and retract filament before auto Z offset. The default is `0` (off).

## Building

The project uses [PlatformIO](https://platformio.org/). The easiest way to build is VS Code with the **Auto Build Marlin** extension.

### Required: Anycubic's STM32 framework

This firmware needs **Anycubic's modified `framework-arduinoststm32` (version 4.10900.200819)**, not the stock package PlatformIO downloads. A build against the stock framework compiles without errors, but it uses a generic board layout and the printer won't take it.

Anycubic's version adds the `AC_TRI_F103RE` board variant (pin map, a linker script that places the firmware at `0x08009000`, stack and heap sizes). It also changes the ADC setup (DMA for temperature readings), flash write timing, and the SD card clock and timeouts.

Install it by replacing the package folder:

| OS | Location |
|---|---|
| Linux / macOS | `~/.platformio/packages/framework-arduinoststm32` |
| Windows | `C:\Users\<you>\.platformio\packages\framework-arduinoststm32` |

To confirm the right one is installed, check that the folder has a `variants/AC_TRI_F103RE` folder and that its `README.md` ends with `framework-arduinoststm32_kobra2_neo`.

### With Auto Build Marlin (recommended)

1. Install [VS Code](https://code.visualstudio.com/).
2. Install the **PlatformIO IDE** extension (`platformio.platformio-ide`) and the **Auto Build Marlin** extension (`marlinfirmware.auto-build`). VS Code suggests both when you open this folder, from `.vscode/extensions.json`.
3. Open the repository root (the folder with `platformio.ini`) in VS Code. Let PlatformIO finish installing its toolchains the first time.
4. Open Auto Build Marlin from the activity bar on the left (the Marlin "M" icon).
5. It reads the board from `Configuration.h` and offers the `ac_tri_f1` environment. Click **Build**.
6. The firmware is written to:
   ```
   .pio/build/ac_tri_f1/firmware.bin
   ```

If Auto Build Marlin lists more than one environment, choose `ac_tri_f1`. It is also the `default_envs` in `platformio.ini`.

### From the command line

With PlatformIO Core installed (`pip install platformio`):

```sh
pio run -e ac_tri_f1
```

To start over with a clean build:

```sh
pio run -e ac_tri_f1 -t clean
```

The output is the same `.pio/build/ac_tri_f1/firmware.bin`.

## Flashing

1. Format a microSD card as FAT32.
2. Copy `firmware.bin` to the root of the card. Keep the name `firmware.bin`, because that's the file the bootloader looks for.
3. Insert the card and reboot the printer. The bootloader flashes the file during startup. Wait until the main screen appears.
4. After flashing a new build, check the settings. Run auto-leveling and Z offset calibration again if anything looks off.

To go back to stock, flash Anycubic's firmware the same way.

## OctoPrint setup

Most slicers don't add `M75`/`M77` to G-code. Have OctoPrint send them so the printer shows the printing screen.

**OctoPrint → Settings → GCODE Scripts**

| Script | Content |
|---|---|
| Before print job starts | `M75` |
| After print job completes | `M77` |
| After print job is cancelled | `M77 C` |
| After print job is paused | `M76` |
| Before print job is resumed | `M75` |

**Progress bar:** use one of these, not both:
- Turn on your slicer's remaining-time (`M73`) output in the printer profile (OrcaSlicer, PrusaSlicer and others have this option). This is the more accurate choice.
- Install an OctoPrint plugin that sends `M73` during the print, such as *M73 Progress*.

Without `M73` the print runs normally, but the progress bar stays empty.

**Serial connection:** `115200` baud. To use a different speed, send `M575 B<rate>` and then reconnect OctoPrint at the new rate.

## Configuration

| File | What it holds |
|---|---|
| [`Marlin/Configuration.h`](Marlin/Configuration.h) | Machine, serial port, baud rate, probing |
| [`Marlin/Configuration_adv.h`](Marlin/Configuration_adv.h) | Buffers, host actions, MeatPack, `M73`, arc support |
| [`Marlin/src/lcd/tft/ui_320x240.cpp`](Marlin/src/lcd/tft/ui_320x240.cpp) | Status screen layout and colors |
| [`Marlin/src/lcd/marlinui.cpp`](Marlin/src/lcd/marlinui.cpp) | Host print start/end, pause and abort handling |
| [`Marlin/src/gcode/stats/M75-M78.cpp`](Marlin/src/gcode/stats/M75-M78.cpp) | `M75`/`M76`/`M77` hooks for host prints |
| [`Marlin/src/HAL/STM32/autoGetZoffset.h`](Marlin/src/HAL/STM32/autoGetZoffset.h) | Auto Z offset settings, including the purge option |
| [`ini/stm32f1.ini`](ini/stm32f1.ini) | `ac_tri_f1` build environment |
| [`buildroot/share/PlatformIO/boards/ac_tri_f103re.json`](buildroot/share/PlatformIO/boards/ac_tri_f103re.json) | Board definition that selects the Anycubic `AC_TRI_F103RE` variant |

## Credits

- [Marlin Firmware](https://github.com/MarlinFirmware/Marlin)
- Anycubic, for the Kobra 2 Neo source release
- [that-ambuj/Kobra2_Neo](https://github.com/that-ambuj/Kobra2_Neo), for the host action commands, MeatPack and USB stutter fixes merged here

## License

GPL-3.0, like Marlin. See [LICENSE](LICENSE).

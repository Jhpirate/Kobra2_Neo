/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../inc/MarlinConfig.h"

#if HAS_STATUS_MESSAGE

#include "../gcode.h"
#include "../../lcd/marlinui.h"

/**
 * M117: Set LCD Status Message
 */
void GcodeSuite::M117() {

  if (parser.string_arg && parser.string_arg[0])
    ui.set_status(parser.string_arg, true);
  else
    ui.reset_status();

  // Anycubic slicer output uses M117 to mark the real print start (after heating).
  // Only restart the timer once per print, so an idle M117 doesn't make the printer look busy
  // and later status messages (e.g. per-layer M117 from OctoPrint plugins) don't reset it.
  if (ui.start_print_status && !did_pause_print && !ui.print_start_marked) {
    ui.print_start_marked = true;
    ui.real_duration();
    print_job_timer.reset();
    print_job_timer.start();
  }
	
}

#endif // HAS_STATUS_MESSAGE

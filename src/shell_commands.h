/*
 *  Shell commands header definitions
 *
 *  File: shell_commands.h
 *  Author: b4fThrive
 *  Copyright (c) 2020 b4f.thrive@gmail.com
 *
 *  This software is released under the MIT License.
 *  https://opensource.org/licenses/MIT
 *
 */

#ifndef _SHELL_COMMANDS_ /* _SHELL_COMMANDS_  prevent defined in other */
#define _SHELL_COMMANDS_

#define E_NULL      " 2> /dev/null"
#define BREAK_BLANK " | sed 's, ,\\n,g'"

#define CAT(f)   "cat " + f + E_NULL
#define LS(d)    "ls " + d + E_NULL
#define LS_LA(d) "ls -la " + d + E_NULL
#define RM(d)    "rm " + d + E_NULL
#define RM_R(d)  "rm -r " + d + E_NULL

#define HWMON_DEVS                                                             \
  "ls /sys/class/hwmon/" E_NULL                                                \
  " | sed -e 's, ,\\n,g' -e 's,^h,\\/sys\\/class\\/hwmon\\/h,g'"

#define HDDTEMP_PATH   "whereis hddtemp" E_NULL BREAK_BLANK " | grep bin"
#define HDDTEMP_SED    " | sed -e 's/.*: //' -e 's/.C//'"

#define DISKS         "lsblk -o NAME | grep -vE 'NAME|[0-9]'"
#define DISK_MODEL_PF "lsblk /dev/"
#define DISK_MODEL_SF "-o MODEL,TYPE" E_NULL " | grep disk | sed 's, *disk,,g'"
#define DISK_INFO_PF  "lsblk -o SERIAL,MODEL,NAME" E_NULL " | grep -vE '"
#define DISK_INFO_SF  "[0-9]|SERIAL' | sed 's,  .*$,,g'" BREAK_BLANK
#define DISK_MODEL(d) DISK_MODEL_PF + d + DISK_MODEL_SF

#endif /* _SHELL_COMMANDS_ */

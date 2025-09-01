#ifndef DISKCMD_H
#define DISKCMD_H

#include "command.h"
#include <fujinet-fuji.h>

// Disk test pseudo-commands handled locally by the OS on Apple II.
// These are NOT FUJICMD_*; they are harness-level commands used when
// tc_buf.device is in the DISK device range (0x31..0x3F).
enum {
  DISKCMD_OPEN  = 0x10,
  DISKCMD_READ  = 0x11,
  DISKCMD_WRITE = 0x12,
  DISKCMD_CLOSE = 0x13,
  DISKCMD_SEEK  = 0x14,
  DISKCMD_STAT  = 0x15,
};

// Access flags (mirror the values used in filecmd.h for consistency)
#define DISK_OPEN_READ   0x40
#define DISK_OPEN_WRITE  0x80

extern bool disk_command(TestCommand *cmd, void *data, void *reply, size_t reply_max);

#endif /* DISKCMD_H */

#ifndef DISKCMD_H
#define DISKCMD_H

#include "command.h"
#include <fujinet-fuji.h>

extern bool disk_command(TestCommand *cmd, void *data, void *reply, size_t reply_max);

#endif /* DISKCMD_H */

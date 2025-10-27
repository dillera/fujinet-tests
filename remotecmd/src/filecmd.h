#ifndef FILECMD_H
#define FILECMD_H

#include "command.h"
#ifdef __MSX__
#include <stdio.h> // MSX changes bool typedef in stdio.h so need to include it
#endif /* __MSX__ */
#include <fujinet-fuji.h>

#define OPEN_READ    0x40
#define OPEN_WRITE   0x80

extern bool file_command(TestCommand *cmd, void *data, void *reply, size_t reply_max);

#endif /* FILECMD_H */

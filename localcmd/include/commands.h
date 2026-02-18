#ifndef COMMANDS_H
#define COMMANDS_H

#include <fujinet-fuji.h>

enum {
  TYPE_BOOL      = 'b',
  TYPE_UNSIGNED  = 'u',
  TYPE_FIXED_LEN = 'f',
  TYPE_VAR_LEN   = 's',
  TYPE_STRUCT    = '{',
};

typedef enum {
  ENDIAN_LITTLE  = '<',
  ENDIAN_BIG     = '>',
} ENDIAN;

#ifdef _CMOC_VERSION_
#define ENDIAN_NATIVE ENDIAN_BIG
#else /* ! _CMOC_VERSION_ */
#define ENDIAN_NATIVE ENDIAN_LITTLE
#endif /* _CMOC_VERSION_ */

typedef struct {
  char *name;
  uint8_t type;
  uint8_t endian;
  uint16_t size;
} FujiArg;

typedef struct FujiCommand {
  uint8_t command;
  char *name;
  uint8_t argCount;
  FujiArg *args;
  FujiArg reply;
  struct FujiCommand *next;
} FujiCommand;

extern FN_ERR load_commands(const char *path);
extern FujiCommand *find_command(const char *name);

#endif /* COMMANDS_H */

#include "commands.h"
#include "json.h"

#define malloc(len) sbrk(len)

typedef struct {
  char *name;
  uint8_t type;
  uint16_t size;
} FujiArg;

typedef struct {
  uint8_t command;
  char *name;
  uint8_t argCount;
  FujiArg *args;
  FujiArg reply;
} FujiCommand;

FujiCommand *fuji_commands = 0xAAAA;

uint8_t load_commands(const char *path)
{
  uint8_t err, cnum;
  uint16_t count, idx;
  size_t length;
  char query[32];
  char buffer[64];


  err = json_open(path);
  if (err != FN_ERR_OK)
    return err;

  for (count = 0; ; count++) {
    sprintf(query, "/%d/command", count);
    length = json_query(query, buffer);
    if (!length)
      break;
  }

  printf("Total commands: %d\n", count);

  idx = sizeof(FujiCommand) * count;
  printf("Allocating %d bytes\n", idx);
  fuji_commands = (FujiCommand *) malloc(idx);
  printf("FUJI COMMANDS = %04X\n", fuji_commands);

  for (idx = 0; idx < count; idx++) {
    sprintf(query, "/%d/command", idx);
    length = json_query(query, buffer);
    if (!length)
      break;

    cnum = (uint8_t) atoi(buffer);
    sprintf(query, "/%d/name", idx);
    length = json_query(query, buffer);
    if (!length)
      break;

    fuji_commands[idx].command = cnum;
    fuji_commands[idx].name = (char *) malloc(strlen(buffer) + 1);
    strcpy(fuji_commands[idx].name, buffer);

    printf("IDX %d == 0x%02x %s\n", idx, fuji_commands[idx].command, fuji_commands[idx].name);

    // FIXME - get args & reply
  }

  json_close();

  return FN_ERR_OK;
}

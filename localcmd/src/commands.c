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

void parse_command_arg(FujiArg *arg, const char *buffer)
{
  // FIXME - split on ':'
  // FIXME - fill in type and size fields
  arg->name = (char *) malloc(strlen(buffer) + 1);
  strcpy(arg->name, buffer);
  return;
}

uint8_t load_commands(const char *path)
{
  uint8_t err, cnum;
  uint16_t count, idx, jdx;
  size_t length;
  char query[32];
  char buffer[64];


  err = json_open(path);
  if (err != FN_ERR_OK)
    return err;

  printf("Counting Fuji commands...\n");
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

    fuji_commands[idx].reply.name = NULL;

    sprintf(query, "/%d/reply/0", idx);
    length = json_query(query, buffer);
    if (length)
      parse_command_arg(&fuji_commands[idx].reply, buffer);

    for (jdx = 0; ; jdx++) {
      sprintf(query, "/%d/args/%d", idx, jdx);
      length = json_query(query, buffer);
      if (!length)
        break;
    }

    fuji_commands[idx].argCount = (uint8_t) jdx;
    if (fuji_commands[idx].argCount) {
      fuji_commands[idx].args = (FujiArg *) sbrk(sizeof(FujiArg)
                                                 * fuji_commands[idx].argCount);
      for (jdx = 0; jdx < fuji_commands[idx].argCount; jdx++) {
        sprintf(query, "/%d/args/%d", idx, jdx);
        length = json_query(query, buffer);
        parse_command_arg(&fuji_commands[idx].args[jdx], buffer);
      }
    }
  }

  json_close();

  return FN_ERR_OK;
}

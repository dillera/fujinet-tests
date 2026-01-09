#include "json.h"
#include "commands.h"
#include "testing.h"

#define FLAG_EXCEEDS_U8  0x04
#define FLAG_EXCEEDS_U16 0x02

void add_test_argument(TestCommand *test, FujiArg *arg, const char *input,
                       int *auxpos, void **dataptr)
{
  int val, size, offset;
  uint8_t *ptr;


  switch (arg->type) {
  case 's':
  case 'b':
    printf("Unhandled type %c\n", arg->type);
    exit(1);
    break;

  case 'f':
    test->data_len = arg->size;
    *dataptr = input;
    break;

  default: // integer types
    val = atoi(input); // FIXME - handle unsigned greater than 32767
    ptr = &test->aux1;
    for (size = arg->size, offset = *auxpos; size; size -= 8, offset++) {
      // FXIME - deal with endianness
      *(ptr + offset) = (uint8_t) (val & 0xFF);
      val >>= 8;
    }
    *auxpos = offset;
    test->flags++;
    if (test->flags < 4 && arg->size > 8) {
      test->flags += FLAG_EXCEEDS_U8;
      if (arg->size > 16)
        test->flags += FLAG_EXCEEDS_U16;
    }
    break;
  }

  return;
}

void get_json_data(void)
{
    char command[50];
    int16_t bytesread;
    int count = 0;
    char query[256];
    FujiCommand *cmd;
    uint16_t idx;
    TestCommand test;
    int auxpos;
    void *data, *expected;

    while (true)
    {

        sprintf(query, "/%d/command", count);
        bytesread = json_query(query, command);
        if (!bytesread) // If we didn't get any more objects with "command" in them, we're done.
        {
            break;
        }

        printf("command: %s\n", command);
        cmd = find_command(command);
        if (!cmd) {
          printf("Unknown command\n");
          return;
        }
        printf("found: %s\n", cmd->name);

        sprintf(query, "/%d/device", count);
        bytesread = json_query(query, command);
        if (!bytesread)
          test.device = FUJI_DEVICEID_FUJINET;
        else {
          printf("FIXME - deal with device %s\n", command);
          exit(1);
        }
        test.command = cmd->command;
        test.flags = 0;
        test.aux1 = test.aux2 = test.aux3 = test.aux4 = 0;
        test.data_len = test.reply_len = 0;
        auxpos = 0;
        data = expected = NULL;

        for (idx = 0; idx < cmd->argCount; idx++) {
          sprintf(query, "/%d/%s", count, cmd->args[idx].name);
          bytesread = json_query(query, command);
          if (!bytesread) {
            printf("Argument %s missing\n", cmd->args[idx].name);
            return;
          }
          printf("Arg %s = %s\n", cmd->args[idx].name, command);
          add_test_argument(&test, &cmd->args[idx], command, &auxpos, &data);
        }

        // FIXME - get reply, expected

        sprintf(query, "/%d/warnOnly", count);
        bytesread = json_query(query, command);
        if (bytesread)
          test.flags |= FLAG_WARN;

        sprintf(query, "/%d/errorExpected", count);
        bytesread = json_query(query, command);
        if (bytesread)
          test.flags |= FLAG_EXPERR;

        if (!run_test(&test, data, expected)) {
          printf("TEST FAILED\n");
          return;
        }

        count++;
        waitkey(1);
    }

    printf("\n%d tests read.\n", count);
}

int main(void)
{
    cls(1);

    load_commands("COMMANDS.JSN");

    if (json_open("TESTS.JSN") != FN_ERR_OK) {
      printf("Failed to open JSON file\n");
      return 1;
    }

    printf("Getting json data\n");
    get_json_data();
    json_close();

    return 0;
}

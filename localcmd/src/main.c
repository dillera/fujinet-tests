#include "json.h"
#include "commands.h"

void get_json_data(void)
{
    char command[50];

    int16_t bytesread;
    int count = 0;
    char query[256];

    while (true)
    {

        sprintf(query, "/%d/command", count);
        bytesread = json_query(query, command);
        if (!bytesread) // If we didn't get any more objects with "command" in them, we're done.
        {
            break;
        }

        printf("command: %s\n", command);

        count++;
        waitkey(1);
    }

    printf("\n%d JSON objects read.\n", count);
}

int main(void)
{
    cls(1);

    load_commands("COMMANDSJSN");

    if (json_open("TESTS   JSN") != FN_ERR_OK) {
      printf("Failed to open JSON file\n");
      return 1;
    }

    printf("Getting json data\n");
    get_json_data();
    json_close();

    return 0;
}


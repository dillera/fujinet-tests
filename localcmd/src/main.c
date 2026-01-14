#include "commands.h"
#include "testing.h"
#include "results.h"
#include "platform.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#ifdef BUILD_ATARI
#define exit(x) while(1)
#define PREFIX "D:"
#else /* ! BUILD_ATARI */
#define PREFIX ""
#endif /* BUILD_ATARI */

int main(void)
{
    init();
    clrscr();

    if (load_commands(PREFIX "COMMANDS.JSN") != FN_ERR_OK) {
      printf("No commands found\n");
      exit(1);
    }

    execute_tests("TESTS.JSN");
    if (screen_width > 32)
    {
        printf("\nTests complete. Press a key for results.");
    }
    else
    {
        printf("\n<Tests complete.>\n");
        printf("Press a key for results.");
    }
    cgetc();
    print_test_results();

    exit(0);
    return 0;
}

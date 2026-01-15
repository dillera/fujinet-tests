#include "commands.h"
#include "testing.h"
#include "results.h"
#include "platform.h"
#include "console.h"

#include <stdlib.h>

#ifdef BUILD_ATARI
#define exit(x) while(1)
#define PREFIX "D:"
#else /* ! BUILD_ATARI */
#define PREFIX ""
#endif /* BUILD_ATARI */

int main(void)
{
    console_init();
    clrscr();

    if (load_commands(PREFIX "COMMANDS.JSN") != FN_ERR_OK) {
      printf("No commands found\n");
      exit(1);
    }

    execute_tests("TESTS.JSN");
    if (console_width > 32)
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
#ifdef _CMOC_VERSION_
    // All other compiles will complain this is unreachable code
    return 0;
#endif /* _CMOC_VERSION_ */
}

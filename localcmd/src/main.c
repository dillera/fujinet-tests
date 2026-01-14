#include "commands.h"
#include "testing.h"
#include "results.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

int main(void)
{
#ifdef _CMOC_VERSION_
    initCoCoSupport();
    if (isCoCo3)
    {
        width(40);
    }
    else
    {
        width(32);
    }
    cls(1);
#else
    clrscr();
#endif /* _CMOC_VERSION_ */

    if (load_commands("COMMANDS.JSN") != FN_ERR_OK) {
      printf("No commands found\n");
      exit(1);
    }

    execute_tests("TESTS.JSN");
#ifdef _CMOC_VERSION_
    if (isCoCo3)
    {
        printf("\nTests complete. Press a key for results.");
    }
    else
    {
        printf("\n<Tests complete.>\n");
        printf("Press a key for results.");
    }
    waitkey(0);
#else
    printf("\nTests complete. Press a key for results.");
    cgetc();
#endif
    print_test_results();

    return 0;
}

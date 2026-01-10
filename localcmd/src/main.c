#include "commands.h"
#include "testing.h"

int main(void)
{
    cls(1);

    load_commands("COMMANDS.JSN");
    printf("Running tests\n");
    execute_tests("TESTS.JSN");

    return 0;
}

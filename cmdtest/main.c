#include "machineid.h"
#include "fujicmd.h"

#include <fujinet-fuji.h>
#ifndef _CMOC_VERSION_
#include <string.h>
#include <stdlib.h>
#else /* _CMOC_VERSION_ */
#include <cmoc.h>
#define cprintf printf
#endif /* _CMOC_VERSION_ */

#ifdef __CC65__
#include <conio.h>
#endif /* __CC65__ */

AdapterConfigExtended ace;

int main()
{
  uint8_t instafail = 0;
  uint8_t machine_type;
  const char *name;


#ifdef _CMOC_VERSION_
  machine_type = 1;
#else
  machine_type = get_ostype();
#endif

#ifndef _CMOC_VERSION_
  clrscr();
#endif
  cprintf("Searching for FujiNet...\r\n");
  if (!fuji_get_adapter_config_extended(&ace)) {
    strcpy(ace.fn_version, "FAIL");
    instafail = 1;
  }

  name = machine_name(machine_type);
  cprintf("Machine: %.s\r\n", name);
  cprintf("FujiNet: %-14s  Make: ???\r\n", ace.fn_version);

  if (instafail)
    exit(1);

  // FIXME - test everything listed in fujicmd.h
  
  cprintf("All tests passed!\n");
  exit(0);
  return 0;
}

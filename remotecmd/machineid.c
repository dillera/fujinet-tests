#include "machineid.h"
#ifndef _CMOC_VERSION_
#include <stdint.h>
#include <stdio.h>
#else /* _CMOC_VERSION */
#include <coco.h>
#endif /* _CMOC_VERSION_ */
#ifdef __APPLE2__
#include <apple2.h>
#endif /* __APPLE2__ */

struct machine_name {
  uint8_t id;
  const char *name;
};

struct machine_name machine_map[] = {
  {0x00, "Unknown computer"},
#ifdef __APPLE2__
  {APPLE_II, "Apple ]["},
  {APPLE_IIPLUS, "Apple ][ Plus"},
  {APPLE_IIJPLUS, "Apple ][ J-Plus"},
  {APPLE_IIIEM, "Apple ///"},
  {APPLE_IIE, "Apple IIe"},
  {APPLE_IIEENH, "Apple IIe (enhanced)"},
  {APPLE_IIECARD, "Apple IIe option card"},
  {APPLE_IIC, "Apple IIc"},
  {APPLE_IIC35, "Apple IIc ROM 0"},
  {APPLE_IICEXP, "Apple IIc ROM 3"},
  {APPLE_IICREV, "Apple IIc ROM 4"},
  {APPLE_IICPLUS, "Apple IIc Plus"},
  {APPLE_IIGS, "Apple IIgs"},
  {APPLE_IIGS1, "Apple IIgs ROM 1"},
  {APPLE_IIGS3, "Apple IIgs ROM 3"},
#endif /* __APPLE2__ */
  {0x00, NULL},
};

const char *machine_name(int machine_id)
{
  int idx;


  for (idx = 0; machine_map[idx].name; idx++)
    if (machine_id == machine_map[idx].id)
      return machine_map[idx].name;

  return machine_map[0].name;
}

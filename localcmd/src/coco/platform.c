#include "platform.h"
#include <coco.h>

const char *platform_name()
{
    if (isCoCo3)
      return "coco 3";
    return "coco 1/2";
}

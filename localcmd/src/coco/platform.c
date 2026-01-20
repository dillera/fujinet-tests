#include "platform.h"
#include <coco.h>

const char *computer_model()
{
    if (isCoCo3)
      return "TRS-80 CoCo 3";
    return "TRS-80 CoCo 1/2";
}

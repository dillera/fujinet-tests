#include "console.h"

// Using _CMOC_VERSION_ instead of BUILD_COCO because might reuse this for lib-xplat

#ifdef _CMOC_VERSION_
#include <coco.h>
#endif /* _CMOC_VERSION_ */

uint8_t console_width = 40;
uint8_t console_height = 24;

void console_init(void)
{
#ifdef _CMOC_VERSION_
    initCoCoSupport();
    if (!isCoCo3)
    {
        console_width = 32;
        console_height = 16;
    }
    width(console_width);
#endif /* _CMOC_VERSION_ */

    return;
}

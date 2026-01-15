#ifndef PLATFORM_H
#define PLATFORM_H

#if defined(BUILD_COCO)
extern const char *platform_name();
#elif defined(BUILD_APPLE2)
#define platform_name() "apple2"
#elif defined(BUILD_ATARI)
#define platform_name() "atari"
#elif defined(BUILD_C64)
#define platform_name() "c64"
#elif defined(BUILD_MSX)
#define platform_name() "msx"
#elif defined(BUILD_MSDOS)
#define platform_name() "msdos"
#endif /* BUILD_COCO */

#endif /* PLATFORM_H */

#ifndef PLATFORM_H
#define PLATFORM_H

// platform is the platform that the FujiNet firmware is built for,
// not the exact computer model the test is running on
#if defined(BUILD_COCO)
#define platform_name() "coco"
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
#elif defined(BUILD_ADAM_CPM) || defined(BUILD_ADAM)
#define platform_name() "adam"
#endif

#if defined(BUILD_COCO)
extern const char *computer_model();
#else /* ! BUILD_COCO */
// FIXME - make nicer strings for other computers too
#define computer_model() platform_name()
#endif /* BUILD_COCO */

#endif /* PLATFORM_H */

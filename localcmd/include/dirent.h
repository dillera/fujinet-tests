#ifndef DIRENT_H
#define DIRENT_H

#ifdef _CMOC_VERSION_

#include <disk.h>

#define DECB_SECTOR_SIZE 256

typedef struct {
  byte in_use;
  word offset;
  byte sector;
  byte buffer[DECB_SECTOR_SIZE]; // FIXME - make a pointer with malloc/free
} DIR;

struct dirent {
  char d_name[8 + 3 + 1 + 1]; // name + extension + dot + null terminator
};

extern DIR *opendir(const char *name);
extern int closedir(DIR *dirp);
extern struct dirent *readdir(DIR *dirp);

#else /* ! _CMOC_VERSION_ */

#include <dirent.h>

#endif /* _CMOC_VERSION_ */

#endif /* DIRENT_H */

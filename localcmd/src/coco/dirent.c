#include "dirent.h"
#include <coco.h>

#define MAX_OPEN_DIRS 1
#define FIRST_DIR_SECTOR 3
#define LAST_DIR_SECTOR 18
#define DIRECTORY_TRACK 17
#define END_OF_DIR 0xFF
#define ERASED_ENTRY 0x00

typedef struct {
  byte filename[8];
  byte extension[3];
  byte file_type;
  byte ascii_flag;
  byte first_granule;
  word last_sector_len;
  byte reserved[16];
} decb_dir_entry;

static DIR dirtable[MAX_OPEN_DIRS];
static struct dirent cur_entry;

static byte read_next_dir_sector(DIR *dirp)
{
  if (dirp->sector > LAST_DIR_SECTOR)
    return 0;
  dirp->offset = 0;
  readDiskSector(dirp->buffer, curDriveNo, DIRECTORY_TRACK, dirp->sector);
  dirp->sector++;
  return 1;
}

DIR *opendir(const char *name)
{
  byte idx;


  for (idx = 0; idx < MAX_OPEN_DIRS; idx++) {
    if (!dirtable[idx].in_use)
      break;
  }

  if (idx == MAX_OPEN_DIRS)
    return NULL;

  dirtable[idx].in_use = 1;
  dirtable[idx].sector = FIRST_DIR_SECTOR;
  read_next_dir_sector(&dirtable[idx]);

  return &dirtable[idx];
}

int closedir(DIR *dirp)
{
  dirp->in_use = 0;
  return 0;
}

// Converts an 11-byte normalized filename back into a readable string.
// Input format: 8 chars name + 3 chars extension (space padded), null-terminated.
// Output: "FILENAME.EXT" or "FILENAME" if no extension.
// Does NOT restore original case.
//
void denormalize(char *dest, const char *src)
{
    int i;
    int end;

    // Copy filename (trim trailing spaces)
    end = 7;
    while (end >= 0 && src[end] == ' ')
        --end;

    for (i = 0; i <= end; ++i)
        *dest++ = src[i];

    // Copy extension if present (trim trailing spaces)
    src += 8;
    end = 2;
    while (end >= 0 && src[end] == ' ')
        --end;

    if (end >= 0)
    {
        *dest++ = '.';
        for (i = 0; i <= end; ++i)
            *dest++ = src[i];
    }

    *dest = '\0';
}

struct dirent *readdir(DIR *dirp)
{
  decb_dir_entry *entry;


  while (1) {
    while (dirp->offset < DECB_SECTOR_SIZE) {
      entry = (decb_dir_entry *) &dirp->buffer[dirp->offset];
      dirp->offset += sizeof(decb_dir_entry);

      if (entry->filename[0] == ERASED_ENTRY)
        continue;
      if (entry->filename[0] == END_OF_DIR)
        return NULL;

      memcpy(cur_entry.d_name, entry->filename,
             sizeof(entry->filename) + sizeof(entry->extension));
      denormalize(cur_entry.d_name, (char *) entry->filename);
      return &cur_entry;
    }

    read_next_dir_sector(dirp);
  }

  return NULL; // Never reaches here
}

#include "stdio.h"
#include <disk.h>
#include <fujinet-fuji.h> // for bool

typedef struct {
  bool is_open;
  struct FileDesc fd;
} stdio_handle;

int errno = 0;

static bool stdio_initialized = false;
static byte fat_buffer[MAX_NUM_GRANULES];
static stdio_handle stdio_fd[MAX_STDIO_OPEN_FILES];

static void stdio_init()
{
  if (stdio_initialized)
    return;
  memset(stdio_fd, 0, sizeof(stdio_fd));
  initdisk(fat_buffer);
  stdio_initialized = true;
  return;
}

// Returns index into stdio_fd if handle is valid, else returns -1
int stdio_validate_handle(FILE *stream)
{
  int idx;


  if (!stream)
    return -1;
  idx = ((int) (stream)) - 1;
  if (idx >= MAX_STDIO_OPEN_FILES || !stdio_fd[idx].is_open)
    return -1;
  return idx;
}

FILE *fopen(const char *pathname, const char *mode)
{
  int idx;


  stdio_init();

  for (idx = 0; idx < MAX_STDIO_OPEN_FILES; idx++) {
    if (!stdio_fd[idx].is_open) {
      if (!openfile(&stdio_fd[idx].fd, pathname)) {
        // FIXME - set errno?
        return NULL;
      }

      // FIXME - do something with mode?
      stdio_fd[idx].is_open = true;
      return (FILE *) (idx + 1);
    }
  }

  return NULL;
}

int fclose(FILE *stream)
{
  int idx;


  idx = stdio_validate_handle(stream);
  if (idx < 0)
    return -1;

  close(&stdio_fd[idx].fd);
  stdio_fd[idx].is_open = false;
  return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  int idx;
  word length;


  idx = stdio_validate_handle(stream);
  if (idx < 0)
    return -1;

  // FIXME - what if size * nmemb > 32767
  length = read(&stdio_fd[idx].fd, (char *) ptr, size * nmemb);
  return length / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  printf("fwrite NOT IMPLEMENTED\n");
  exit(1);
  return 0;
}

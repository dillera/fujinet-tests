#include "diskcmd.h"

#ifndef _CMOC_VERSION_
#include <stdint.h>
#include <string.h>
#include <errno.h>
#ifdef __APPLE2__
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#else
#include <stdio.h>
#endif
#endif

#define MAX_OPEN_FILES 8
#ifdef __APPLE2__
static int disk_handles[MAX_OPEN_FILES];
#else
static FILE *disk_handles[MAX_OPEN_FILES];
#endif

static int8_t avail_handle()
{
  for (int i = 0; i < MAX_OPEN_FILES; i++)
    if (!disk_handles[i])
      return i;
  return -1;
}

static bool valid_handle(int8_t fh)
{
  if (fh < 0 || fh >= MAX_OPEN_FILES)
    return false;
  #ifdef __APPLE2__
  return disk_handles[fh] > 0;
  #else
  return disk_handles[fh] != NULL;
  #endif
}

static void u32_to_le(uint32_t v, uint8_t out[4])
{
  out[0] = (uint8_t)(v & 0xFF);
  out[1] = (uint8_t)((v >> 8) & 0xFF);
  out[2] = (uint8_t)((v >> 16) & 0xFF);
  out[3] = (uint8_t)((v >> 24) & 0xFF);
}

static uint32_t le_to_u32(const uint8_t in[4])
{
  return (uint32_t)in[0]
       | ((uint32_t)in[1] << 8)
       | ((uint32_t)in[2] << 16)
       | ((uint32_t)in[3] << 24);
}

bool disk_command(TestCommand *cmd, void *data, void *reply, size_t reply_max)
{
  int8_t fh;
  size_t n;

  switch (cmd->command) {
  case DISKCMD_OPEN:
    // data: zero-terminated path
    // aux1: flags (DISK_OPEN_READ/DISK_OPEN_WRITE)
    fh = avail_handle();
    if (fh < 0)
      return false;
    #ifdef __APPLE2__
    {
      int flags = 0;
      if (cmd->aux1 == DISK_OPEN_READ)
        flags = O_RDONLY | O_BINARY;
      else if (cmd->aux1 == DISK_OPEN_WRITE)
        flags = O_WRONLY | O_CREAT | O_TRUNC | O_BINARY;
      else if (cmd->aux1 == (DISK_OPEN_READ | DISK_OPEN_WRITE))
        flags = O_RDWR | O_CREAT | O_BINARY;
      else
        return false;
      int fd = open((const char*)data, flags, 0666);
      if (fd < 0)
        return false;
      disk_handles[fh] = fd;
      ((int8_t *) reply)[0] = fh;
      return true;
    }
    #else
    {
      const char *mode;
      if (cmd->aux1 == DISK_OPEN_READ)
        mode = "rb";
      else if (cmd->aux1 == DISK_OPEN_WRITE)
        mode = "wb";
      else if (cmd->aux1 == (DISK_OPEN_READ | DISK_OPEN_WRITE))
        mode = "rb+";
      else
        return false;
      disk_handles[fh] = fopen((const char*)data, mode);
      if (!disk_handles[fh])
        return false;
      ((int8_t *) reply)[0] = fh;
      return true;
    }
    #endif

  case DISKCMD_READ:
    // aux1: fh
    if (!valid_handle(cmd->aux1))
      return false;
    // reply_len indicates desired bytes; cap to reply_max
    #ifdef __APPLE2__
    {
      size_t want = (cmd->reply_len < reply_max) ? cmd->reply_len : reply_max;
      int r = read(disk_handles[cmd->aux1], reply, want);
      if (r < 0) return false;
      cmd->reply_len = (uint16_t)r;
      return true;
    }
    #else
    {
      n = fread(reply, 1, (cmd->reply_len < reply_max) ? cmd->reply_len : reply_max, disk_handles[cmd->aux1]);
      cmd->reply_len = (uint16_t)n; // return actual amount read
      return true;
    }
    #endif

  case DISKCMD_WRITE:
    // aux1: fh
    if (!valid_handle(cmd->aux1))
      return false;
    // write data_len bytes from data
    #ifdef __APPLE2__
    {
      int w = write(disk_handles[cmd->aux1], data, cmd->data_len);
      return w == (int)cmd->data_len;
    }
    #else
    {
      n = fwrite(data, 1, cmd->data_len, disk_handles[cmd->aux1]);
      return n == cmd->data_len;
    }
    #endif

  case DISKCMD_SEEK:
    // aux1: fh
    // data: [0..3]=uint32 offset LE, [4]=whence (0=SET,1=CUR,2=END)
    if (!valid_handle(cmd->aux1))
      return false;
    if (cmd->data_len < 5)
      return false;
    {
      uint8_t *b = (uint8_t*)data;
      uint32_t off = le_to_u32(&b[0]);
      int whence = (int)b[4];
      #ifdef __APPLE2__
      long res = lseek(disk_handles[cmd->aux1], (long)off, whence);
      return res >= 0;
      #else
      if (fseek(disk_handles[cmd->aux1], (long)off, whence) != 0)
        return false;
      return true;
      #endif
    }

  case DISKCMD_STAT:
    // aux1: fh
    // reply: 4 bytes size (LE)
    if (!valid_handle(cmd->aux1))
      return false;
    {
      #ifdef __APPLE2__
      struct stat st;
      if (fstat(disk_handles[cmd->aux1], &st) != 0)
        return false;
      if (reply_max < 4) return false;
      u32_to_le((uint32_t)st.st_size, (uint8_t*)reply);
      cmd->reply_len = 4;
      return true;
      #else
      long cur = ftell(disk_handles[cmd->aux1]);
      if (cur < 0) cur = 0;
      if (fseek(disk_handles[cmd->aux1], 0L, SEEK_END) != 0)
        return false;
      long end = ftell(disk_handles[cmd->aux1]);
      if (end < 0) end = 0;
      if (fseek(disk_handles[cmd->aux1], cur, SEEK_SET) != 0)
        return false;
      if (reply_max < 4)
        return false;
      u32_to_le((uint32_t)end, (uint8_t*)reply);
      cmd->reply_len = 4;
      return true;
      #endif
    }

  case DISKCMD_CLOSE:
    // aux1: fh
    if (!valid_handle(cmd->aux1))
      return false;
    #ifdef __APPLE2__
    if (close(disk_handles[cmd->aux1]) != 0)
      return false;
    disk_handles[cmd->aux1] = -1;
    return true;
    #else
    if (fclose(disk_handles[cmd->aux1]) != 0)
      return false;
    disk_handles[cmd->aux1] = NULL;
    return true;
    #endif
  }

  return false;
}

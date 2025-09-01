#include "diskcmd.h"

#ifndef _CMOC_VERSION_
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#endif

#define MAX_OPEN_FILES 8
static FILE *disk_handles[MAX_OPEN_FILES];

static int8_t avail_handle()
{
  int i;
  for (i = 0; i < MAX_OPEN_FILES; i++)
    if (!disk_handles[i])
      return i;
  return -1;
}

static bool valid_handle(int8_t fh)
{
  if (fh < 0 || fh >= MAX_OPEN_FILES)
    return false;
  return disk_handles[fh] != NULL;
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
  const char *mode;
  size_t n;

  switch (cmd->command) {
  case DISKCMD_OPEN:
    // data: zero-terminated path
    // aux1: flags (DISK_OPEN_READ/DISK_OPEN_WRITE)
    if (cmd->aux1 == DISK_OPEN_READ)
      mode = "rb";
    else if (cmd->aux1 == DISK_OPEN_WRITE)
      mode = "wb";
    else if (cmd->aux1 == (DISK_OPEN_READ | DISK_OPEN_WRITE))
      mode = "rb+";
    else
      return false;

    fh = avail_handle();
    if (fh < 0)
      return false;

    disk_handles[fh] = fopen((const char*)data, mode);
    if (!disk_handles[fh])
      return false;

    ((int8_t *) reply)[0] = fh;
    return true;

  case DISKCMD_READ:
    // aux1: fh
    if (!valid_handle(cmd->aux1))
      return false;
    // reply_len indicates desired bytes; cap to reply_max
    n = fread(reply, 1, (cmd->reply_len < reply_max) ? cmd->reply_len : reply_max, disk_handles[cmd->aux1]);
    cmd->reply_len = (uint16_t)n; // return actual amount read
    return true;

  case DISKCMD_WRITE:
    // aux1: fh
    if (!valid_handle(cmd->aux1))
      return false;
    // write data_len bytes from data
    n = fwrite(data, 1, cmd->data_len, disk_handles[cmd->aux1]);
    // We don't currently return count; treat partial as failure
    return n == cmd->data_len;

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
      if (fseek(disk_handles[cmd->aux1], (long)off, whence) != 0)
        return false;
      return true;
    }

  case DISKCMD_STAT:
    // aux1: fh
    // reply: 4 bytes size (LE)
    if (!valid_handle(cmd->aux1))
      return false;
    {
      long cur, size;
      cur = ftell(disk_handles[cmd->aux1]);
      if (cur < 0) cur = 0;
      if (fseek(disk_handles[cmd->aux1], 0L, SEEK_END) != 0)
        return false;
      size = ftell(disk_handles[cmd->aux1]);
      if (size < 0) size = 0;
      if (fseek(disk_handles[cmd->aux1], cur, SEEK_SET) != 0)
        return false;
      if (reply_max < 4)
        return false;
      u32_to_le((uint32_t)size, (uint8_t*)reply);
      cmd->reply_len = 4;
      return true;
    }

  case DISKCMD_CLOSE:
    // aux1: fh
    if (!valid_handle(cmd->aux1))
      return false;
    if (fclose(disk_handles[cmd->aux1]) != 0)
      return false;
    disk_handles[cmd->aux1] = NULL;
    return true;
  }

  return false;
}

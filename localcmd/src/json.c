#include "json.h"
#include <fujinet-network.h>

// FIXME - use decbfile library to read JSON in small chunks?
// https://gvlsywt.cluster051.hosting.ovh.net/dev/decbfile-0.1.10.tar.gz
static byte decb_buffer[256];
static byte json_contents[8192];

#define PORT "7501"
#define WRITE_SOCKET "N1:TCP://:" PORT
#define READ_SOCKET "N2:TCP://localhost:" PORT

uint8_t json_open(const char *path)
{
  size_t length;
  uint8_t err, status;
  uint16_t avail;


  err = readDECBFileWithDECB(json_contents, 0, path, decb_buffer, &length);
  if (err != 0) {
    // Error reading file
    switch (err) {
    case 1:
      printf("Error: Read error reading directory sectors.\n");
      break;
    case 2:
      printf("Error: File not found.\n");
      break;
    case 3:
      printf("Error: Read error reading the FAT.\n");
      break;
    case 4:
      printf("Error: Read error reading a file sector.\n");
    default:
      printf("Error: Unknown error code %d.\n", err);
      break;
    }
    return FN_ERR_IO_ERROR;
  }
  printf("Read %u bytes from %s\n", length, path);

  printf("Opening write socket (%s)...\n", WRITE_SOCKET);
  err = network_open(WRITE_SOCKET, OPEN_MODE_RW, 0);
  if (err != FN_ERR_OK)
    return err;

  printf("Opening read socket (%s)...\n", READ_SOCKET);
  err = network_open(READ_SOCKET, OPEN_MODE_READ, 0);
  if (err != FN_ERR_OK)
    return err;

  for (;;) {
    err = network_status(WRITE_SOCKET, &avail, &status, &err);
    // printf("AVAIL: %u  STATUS: %u  ERR: %u\n", avail, status, err);
    if (err != FN_ERR_OK)
      return err;
    if (status == 1)
      break;
  }

  err = network_accept(WRITE_SOCKET);
  if (err != FN_ERR_OK)
    return err;

  err = network_write(WRITE_SOCKET, json_contents, length);
  network_close(WRITE_SOCKET);
  network_json_parse(READ_SOCKET);

  return FN_ERR_OK;
}

void json_close()
{
  network_close(READ_SOCKET);
  return;
}

size_t json_query(const char *query, void *buffer)
{
  return network_json_query(READ_SOCKET, query, (char *) buffer);
}

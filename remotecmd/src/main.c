#include "deviceid.h"
#include "diskcmd.h"
#include "filecmd.h"
#include "hexdump.h"
#include "endian.h"

#include <fujinet-fuji.h>
#include <fujinet-network.h>

#ifndef _CMOC_VERSION_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#endif /* _CMOC_VERSION_ */

#define FLAG_WARN 0x10
#define LAST_CONTROLLER "UNITRIX.TXT"

AdapterConfigExtended ace;
TestCommand tc_buf;
uint8_t buffer[256];
char controller[256];

#define MIDPOINT (sizeof(controller) / 2)

void read_string(char *buffer, int buflen)
{
#ifndef _CMOC_VERSION_
  fgets(buffer, buflen, stdin);
#else
  int key, idx;


  for (idx = 0; idx < buflen - 1; idx++) {
    while (true) {
      key = inkey();
      if (key)
        break;
    }

    if (key == '\r' || key == '\n')
      break;

    if (key == '\b') {
      idx--;
      if (idx) {
        printf("\b \b");
        idx--;
      }
      continue;
    }

    putchar(key);

    buffer[idx] = (char) key;
  }
  buffer[idx] = 0;
  printf("\n");
#endif /* _CMOC_VERSION_ */
  return;
}

int main()
{
  uint8_t *data, *reply;
  size_t datalen;
  int16_t rlen, wlen;
  bool success, did_fail;
  uint16_t avail;
  uint8_t status, err;
#ifndef _CMOC_VERSION_
  FILE *file;
#endif /* _CMOC_VERSION_ */
  uint8_t fail_count = 0;


  if (!fuji_get_adapter_config_extended(&ace)) {
    strcpy(ace.fn_version, "FAIL");
    fail_count++;
  }

  printf("FujiNet: %-14s  Make: ???\n", ace.fn_version);

  if (fail_count)
    exit(1);

  buffer[0] = 0;
#ifndef _CMOC_VERSION_
  file = fopen(LAST_CONTROLLER, "r");
  if (file) {
    fgets((char *) buffer, sizeof(buffer), file);
    rlen = strlen((char *) buffer);
    while (buffer[rlen-1] == '\n')
      rlen--;
    buffer[rlen] = 0;
    fclose(file);
  }
#endif /* _CMOC_VERSION_ */

  printf("Hostname/IP address of test controller? ");
  if (buffer[0])
    printf("[%s] ", buffer);

  // Why allocate a second buffer when I can use the back half of the buffer I have?
  read_string(&controller[MIDPOINT], sizeof(controller) - MIDPOINT);

  rlen = strlen(&controller[MIDPOINT]);
  while (controller[MIDPOINT+rlen-1] == '\n')
    rlen--;
  controller[MIDPOINT+rlen] = 0;
  if (!controller[MIDPOINT])
    strcpy(&controller[MIDPOINT], (char *) buffer);

#ifndef _CMOC_VERSION_
  file = fopen(LAST_CONTROLLER, "w");
  if (file) {
    fprintf(file, "%s\n", &controller[MIDPOINT]);
    fclose(file);
  }
#endif /* _CMOC_VERSION_ */

  sprintf(controller, "N:TCP://%s:7357", &controller[MIDPOINT]);

  printf("Opening controller %s\n", controller);
  err = network_open(controller, OPEN_MODE_RW, 0);
  printf("Connection: %d\n", err);
  if (err != FN_ERR_OK) {
    printf("Unable to open connection to test controller: %d\n", err);
    exit(1);
  }

  // Write the AdapterConfigExtended
  wlen = network_write(controller, &ace, sizeof(ace));

  for (;;) {
    printf("Awaiting test\n");
    did_fail = 0;
    // Read command, flags, and 4 aux bytes,
    rlen = network_read(controller, (unsigned char *) &tc_buf, sizeof(tc_buf));
    if (rlen != sizeof(tc_buf)) {
      if (rlen) {
        did_fail = 1;
        fail_count++;
        printf("Network error: %d\n", rlen);
      }
      break;
    }

    tc_buf.data_len = fntohs(tc_buf.data_len);
    tc_buf.reply_len = fntohs(tc_buf.reply_len);

    printf("Received command: 0x%02x:%02x\n"
           "  FLAGS: 0x%02x\n"
	   "  AUX: 0x%02x 0x%02x 0x%02x 0x%02x\n"
	   "  DATA: %d REPLY: %d\n",
	   tc_buf.device, tc_buf.command,
           tc_buf.flags,
	   tc_buf.aux1, tc_buf.aux2, tc_buf.aux3, tc_buf.aux4,
	   tc_buf.data_len, tc_buf.reply_len);

#ifdef UNUSED
    hexdump(&tc_buf, sizeof(tc_buf));
    exit(1);
#endif /* UNUSED */

    datalen = 0;
    reply = data = NULL;
    if (tc_buf.data_len) {
      for (;;) {
	// Wait for data to become available
	if (network_status(controller, &avail, &status, &err))
	  break;
	if (avail)
	  break;
      }
      datalen = network_read(controller, buffer, tc_buf.data_len);
      data = buffer;
      printf("Received data of length: %d\n", datalen);
      if (datalen != tc_buf.data_len) {
	printf("expected %d\n", tc_buf.data_len);
        did_fail = 1;
	fail_count++;
	break;
      }
    }

    if (tc_buf.reply_len)
      reply = buffer;

    printf("Executing 0x%02x:%02x\n", tc_buf.device, tc_buf.command);
    if (tc_buf.device >= FUJI_DEVICEID_DISK && tc_buf.device <= FUJI_DEVICEID_DISK_LAST) {
      // Disks are handled by the opearating system
      success = disk_command(&tc_buf, data, reply, sizeof(buffer));
    }
    else if (tc_buf.device == FUJI_DEVICEID_FILE) {
#ifdef _CMOC_VERSION_
      // CMOC doesn't provide any file I/O routines for DECB
      success = 0;
#else /* ! _CMOC_VERSION_ */
      // pseudo-commands for test controller to open/read/write files
      success = file_command(&tc_buf, data, reply, sizeof(buffer));
#endif /* _CMOC_VERSION_ */
    }
    else {
      success = fuji_bus_call(tc_buf.device, tc_buf.command, tc_buf.flags & 0b11101111,
			      tc_buf.aux1, tc_buf.aux2, tc_buf.aux3, tc_buf.aux4,
			      data, datalen, reply, tc_buf.reply_len);
    }

    if (!(tc_buf.flags & FLAG_WARN) && !success) {
      did_fail = 1;
      fail_count++;
    }

    printf("Success: %d  Fail: %d  Flags: 0x%02x\n", success, did_fail, tc_buf.flags);
    // Send results back to controller
    wlen = network_write(controller, &did_fail, 1);
    if (did_fail)
      break;
    else if (tc_buf.reply_len) {
      printf("Sending reply data: %d\n", tc_buf.reply_len);
      wlen = network_write(controller, reply, tc_buf.reply_len);
    }
  }

  network_close(controller);

  if (fail_count) {
    printf("********************\n");
    printf("%d tests failed\n", fail_count);
    printf("********************\n");

#ifdef __MSX__
  while (1)
    ;
#endif /* __MSX__ */

    exit(1);
  }

  printf("All tests passed!\n");

#ifdef __MSX__
  while (1)
    ;
#endif /* __MSX__ */

  exit(0);
  return 0;
}

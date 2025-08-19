#include "deviceid.h"
#include "diskcmd.h"
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

// FIXME - get config from file or user
#define CONTROLLER "N:TCP://10.4.0.242:7357"
//#define CONTROLLER "N:TCP://10.4.0.102:7357"

#define FLAG_WARN 0x10

AdapterConfigExtended ace;
TestCommand tc_buf;
uint8_t buffer[256];

int main()
{
  uint8_t *data, *reply;
  size_t datalen;
  int16_t rlen, wlen;
  bool success, did_fail;
  uint16_t avail;
  uint8_t status, err;
  uint8_t fail_count = 0;


  if (!fuji_get_adapter_config_extended(&ace)) {
    strcpy(ace.fn_version, "FAIL");
    fail_count++;
  }

  printf("FujiNet: %-14s  Make: ???\n", ace.fn_version);

  if (fail_count)
    exit(1);

  printf("Opening controller\n");
  err = network_open(CONTROLLER, OPEN_MODE_RW, 0);
  printf("Connection: %d\n", err);
  if (err != FN_ERR_OK) {
    printf("Unable to open connection to test controller: %d\n", err);
    exit(1);
  }

  for (;;) {
    did_fail = 0;
    // Read command, flags, and 4 aux bytes,
    rlen = network_read(CONTROLLER, (unsigned char *) &tc_buf, sizeof(tc_buf));
    if (rlen != sizeof(tc_buf)) {
      did_fail = 1;
      fail_count++;
      printf("Network error: %d\n", rlen);
      break;
    }

    tc_buf.data_len = fntohs(tc_buf.data_len);
    tc_buf.reply_len = fntohs(tc_buf.reply_len);

    printf("Received command: 0x%02x:%02x\n"
	   "  AUX: 0x%02x 0x%02x 0x%02x 0x%02x\n"
	   "  DATA: %d REPLY: %d\n",
	   tc_buf.device, tc_buf.command,
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
	if (network_status(CONTROLLER, &avail, &status, &err))
	  break;
	if (avail)
	  break;
      }
      datalen = network_read(CONTROLLER, buffer, tc_buf.data_len);
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
      success = fuji_bus_call(tc_buf.device, 1, tc_buf.command, tc_buf.flags,
			      tc_buf.aux1, tc_buf.aux2, tc_buf.aux3, tc_buf.aux4,
			      data, datalen, reply, tc_buf.reply_len);
    }
    printf("Result: %d 0x%02x\n", success, tc_buf.flags);

    if (!(tc_buf.flags & FLAG_WARN) && !success) {
      did_fail = 1;
      fail_count++;
      break;
    }

    // Send results back to controller
    wlen = network_write(CONTROLLER, &did_fail, 1);
    if (success && tc_buf.reply_len)
      wlen = network_write(CONTROLLER, reply, tc_buf.reply_len);
  }

  network_close(CONTROLLER);

  if (fail_count) {
    printf("********************\n");
    printf("%d tests failed\n", fail_count);
    printf("********************\n");
    exit(1);
  }

  printf("All tests passed!\n");

  exit(0);
  return 0;
}

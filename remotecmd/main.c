#include <fujinet-fuji.h>
#include <fujinet-network.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

// FIXME - get config from file or user
#define CONTROLLER "N:TCP://10.4.0.242:7357"

#define FLAG_DATA	0x10

typedef struct {
  uint8_t command;
  uint8_t flags;
  uint8_t aux1, aux2, aux3, aux4;
  uint16_t reply_len;
} TestCommand;

AdapterConfigExtended ace;
TestCommand tc_buf;
uint8_t buffer[256];

int main()
{
  uint8_t *data, *reply;
  size_t datalen, rlen, wlen;
  bool success;
  uint8_t fail_count = 0;


  if (!fuji_get_adapter_config_extended(&ace)) {
    strcpy(ace.fn_version, "FAIL");
    fail_count++;
  }

  printf("FujiNet: %-14s  Make: ???\n", ace.fn_version);

  if (fail_count)
    exit(1);

  if (network_open(CONTROLLER, OPEN_MODE_RW, 0) != FN_ERR_OK) {
    printf("Unable to open connection to test controller\n");
    exit(1);
  }

  for (;;) {
    // Read command, flags, and 4 aux bytes, 
    rlen = network_read(CONTROLLER, &tc_buf, sizeof(tc_buf));
    printf("Received command: 0x%02x  AUX: 0x%02x 0x%02x 0x%02x 0x%02x  REPLY: %i\n",
	   tc_buf.command, tc_buf.aux1, tc_buf.aux2, tc_buf.aux3, tc_buf.aux4,
	   tc_buf.reply_len);
    if (!rlen)
      break;
    
    datalen = 0;
    reply = data = NULL;
    if (tc_buf.flags & FLAG_DATA) {
      datalen = network_read(CONTROLLER, buffer, sizeof(buffer));
      data = buffer;
      printf("Received data of length: %i\n", datalen);
    }
    if (tc_buf.reply_len)
      reply = buffer;

    success = fuji_bus_call(FUJI_DEVICEID_FUJINET, 1, tc_buf.command, tc_buf.flags,
			    tc_buf.aux1, tc_buf.aux2, tc_buf.aux3, tc_buf.aux4,
			    data, datalen, reply, tc_buf.reply_len);

    if (!success) {
      printf("Command failed: 0x%02x / %i\n", fn_device_error, fn_device_error);
      fail_count++;
      break;
    }
    
    // Send results back to controller
    wlen = network_write(CONTROLLER, &fn_device_error, 1);
    if (success && tc_buf.reply_len)
      wlen = network_write(CONTROLLER, reply, tc_buf.reply_len);
  }

  network_close(CONTROLLER);

  if (fail_count)
    exit(1);

  printf("All tests passed!\n");
  exit(0);
  return 0;
}

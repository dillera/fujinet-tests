#include <fujinet-fuji.h>
#include <fujinet-network.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

// FIXME - get config from file or user
#define CONTROLLER "N:TCP://10.4.0.242:7357"

#define FLAG_WARN 0x10

typedef struct {
  uint8_t command;
  uint8_t flags;
  uint8_t aux1, aux2, aux3, aux4;
  uint16_t data_len, reply_len;
} TestCommand;

AdapterConfigExtended ace;
TestCommand tc_buf;
uint8_t buffer[256];

int main()
{
  uint8_t *data, *reply;
  size_t datalen;
  int16_t rlen, wlen;
  bool success;
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

  if (network_open(CONTROLLER, OPEN_MODE_RW, 0) != FN_ERR_OK) {
    printf("Unable to open connection to test controller\n");
    exit(1);
  }

  for (;;) {
    // Read command, flags, and 4 aux bytes, 
    rlen = network_read(CONTROLLER, &tc_buf, sizeof(tc_buf));
    if (rlen < 0 || fn_device_error)
      break;

    printf("Received command: 0x%02x\n"
	   "  AUX: 0x%02x 0x%02x 0x%02x 0x%02x\n"
	   "  DATA: %i REPLY: %i\n",
	   tc_buf.command,
	   tc_buf.aux1, tc_buf.aux2, tc_buf.aux3, tc_buf.aux4,
	   tc_buf.data_len, tc_buf.reply_len);
    
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
      if (fn_device_error) {
	printf("Failed to read data %i %i %i\n", avail, status, err);
	fail_count++;
	break;
      }
      datalen = network_read(CONTROLLER, buffer, tc_buf.data_len);
      data = buffer;
      printf("Received data of length: %i\n", datalen);
      if (datalen != tc_buf.data_len) {
	printf("expected %i\n", tc_buf.data_len);
	fail_count++;
	break;
      }
    }

    if (tc_buf.reply_len)
      reply = buffer;

    printf("Executing 0x%02x\n", tc_buf.command);
    success = fuji_bus_call(FUJI_DEVICEID_FUJINET, 1, tc_buf.command, tc_buf.flags,
			    tc_buf.aux1, tc_buf.aux2, tc_buf.aux3, tc_buf.aux4,
			    data, datalen, reply, tc_buf.reply_len);
    printf("Result: %i %i\n", success, fn_device_error);

    if (!tc_buf.flags & FLAG_WARN && (!success || fn_device_error)) {
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

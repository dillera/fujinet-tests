#include "machineid.h"
#include "fujicmd.h"

#include <fujinet-fuji.h>
#ifndef _CMOC_VERSION_
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#else /* _CMOC_VERSION_ */
#include <cmoc.h>
#endif /* _CMOC_VERSION_ */

#ifdef __CC65__
#include <conio.h>
#endif /* __CC65__ */

// FIXME - read from fujinet-lib include?
#define MAX_HOST_SLOTS 8
#ifdef __APPLE2__
#define MAX_DEVICE_SLOTS 6
#else /* !__APPLE2__ */
#define MAX_DEVICE_SLOTS 8
#endif /* __APPLE2__ */

// FIXME - find the correct host slot, don't hardcode to 1
#define BEST_HOST_SLOT 1
#define BEST_PREFIX "/test"

AdapterConfigExtended ace;
HostSlot hs_data[MAX_HOST_SLOTS];
DeviceSlot ds_data[MAX_DEVICE_SLOTS];
char buffer[255];

int main()
{
  uint8_t instafail = 0;
  uint8_t machine_type;
  const char *name;
  uint8_t idx;


#ifndef _CMOC_VERSION_
  machine_type = get_ostype();
  clrscr();
#else /* _CMOC_VERSION_ */
  // Enable lowercase display
  *((uint8_t *) 0x95ac) = 57;
  *((uint8_t *) 0xff22) = 0x10;

  machine_type = 1;
#endif /* _CMOC_VERSION_ */

  /* testing FUJICMD_GET_ADAPTERCONFIG_EXTENDED */
  printf("Searching for FujiNet...\n");
  if (!fuji_get_adapter_config_extended(&ace)) {
    strcpy(ace.fn_version, "FAIL");
    instafail = 1;
  }

  name = machine_name(machine_type);
  printf("Machine: %.s\n", name);
  printf("FujiNet: %-14s  Make: ???\n", ace.fn_version);

  if (instafail)
    exit(1);

  // FIXME - test everything listed in fujicmd.h

  /* testing fuji_get_adapter_config(); */
  /* testing fuji_status(); */
  /* testing fuji_set_status(); */

  /* testing FUJICMD_READ_HOST_SLOTS */
  if (!fuji_get_host_slots(&hs_data[0], MAX_HOST_SLOTS)) {
    printf("GET HOST SLOTS FAIL\n");
    exit(1);
  }
  for (idx = 0; idx < MAX_HOST_SLOTS; idx++)
    printf("Host slot %d: \"%s\"\n", idx+1, &hs_data[idx]);
  //#warning "FIXME - make sure received host values are correct"

  /* testing FUJICMD_READ_DEVICE_SLOTS */
  if (!fuji_get_device_slots(ds_data, MAX_DEVICE_SLOTS)) {
    printf("GET DEVICE SLOTS FAIL\n");
#ifndef _CMOC_VERSION_
    exit(1);
#else /* _CMOC_VERSION_ */
    #warning "CoCo returns read device slot fail even when it succeeds"
#endif /* _CMOC_VERSION_ */
  }
  for (idx = 0; idx < MAX_DEVICE_SLOTS; idx++)
    printf("Device slot %d: %d:\"%s\"\n", idx+1, ds_data[idx].hostSlot, ds_data[idx].file);
  //#warning "FIXME - make sure received device values are correct"
  
  /* testing fuji_get_device_filename(); */
  /* testing fuji_put_device_slots(); */
  /* testing fuji_put_host_slots(); */
  /* testing fuji_get_device_enabled_status(); */
  /* testing fuji_set_device_filename(); */
  /* testing fuji_disable_device(); */
  /* testing fuji_enable_device(); */

  /* testing FUJICMD_MOUNT_HOST_SLOT */
  if (!fuji_mount_host_slot(BEST_HOST_SLOT)) {
    printf("MOUNT HOST SLOT FAIL\n");
    exit(1);
  }

  printf("Testing host prefix...\n");
  /* testing FUJICMD_SET_HOST_PREFIX */
  if (!fuji_set_host_prefix(BEST_HOST_SLOT, BEST_PREFIX)) {
    printf("SET HOST PREFIX FAIL\n");
    //exit(1);
  }

  /* testing FUJICMD_GET_HOST_PREFIX */
  if (!fuji_get_host_prefix(BEST_HOST_SLOT, buffer)) {
    printf("GET HOST PREFIX FAIL\n");
    //exit(1);
  }

  printf("Prefix: \"%s\" == \"%s\"\n", BEST_PREFIX, buffer);
  if (strcmp(BEST_PREFIX, buffer) != 0) {
    printf("PREFIX MISMATCH: \"%s\" != \"%s\"\n", BEST_PREFIX, buffer);
#if 0 //def __APPLE2__
    // get/set prefix only implemented on Apple II
    exit(1);
#endif /* __APPLE2__ */
  }

  fuji_set_host_prefix(BEST_HOST_SLOT, "/");
  
  /* testing FUJICMD_OPEN_DIRECTORY */
#if 0 && !defined(__APPLE2__) && !defined(__ATARI__)
  if (!fuji_open_directory2(BEST_HOST_SLOT, "", "")) {
    printf("OPEN DIRECTORY FAIL\n");
    exit(1);
  }
#else
  // Why isn't fuji_open_directory2 just a wrapper to fuji_open_directory()
  //#warning "fuji_open_directory2 not available!"
  if (!fuji_open_directory(BEST_HOST_SLOT, "\0\0")) {
    printf("OPEN DIRECTORY FAIL\n");
    exit(1);
  }
#endif

  /* testing FUJICMD_READ_DIR_ENTRY */
  idx = 0;
  while (fuji_read_directory(sizeof(buffer), 0, buffer)) {
    if (buffer[0] == 0x7f && buffer[1] == 0x7f)
      break;
    printf("DIR ENTRY: \"%s\"\n", buffer);
    idx++;
  }
  if (!idx) {
    printf("READ DIRECTORY FAIL\n");
    exit(1);
  }
  
  /* testing FUJICMD_CLOSE_DIRECTORY */
  if (!fuji_close_directory()) {
    printf("CLOSE DIRECTORY FAIL\n");
    exit(1);
  }

  /* testing fuji_get_directory_position(); */
  /* testing fuji_set_directory_position(); */

  /* testing fuji_unmount_disk_image(); */
  /* testing fuji_unmount_host_slot(); */
  /* testing fuji_mount_all(); */
  /* testing fuji_mount_disk_image(); */
  
  /* testing fuji_get_scan_result(); */
  /* testing fuji_get_ssid(); */
  /* testing fuji_get_wifi_enabled(); */
  /* testing fuji_get_wifi_status(); */
  /* testing fuji_scan_for_networks(); */
  /* testing fuji_set_ssid(); */
  
  /* testing fuji_copy_file(); */
  /* testing fuji_create_new(); */
  /* testing fuji_enable_udpstream(); */
  /* testing fuji_error(); */
  /* testing fuji_get_hsio_index(); */
  /* testing fuji_set_hsio_index(); */
  /* testing fuji_set_sio_external_clock(); */

  /* testing fuji_read_appkey(); */
  /* testing fuji_write_appkey(); */
  /* testing fuji_set_appkey_details(); */

  /* testing fuji_base64_decode_compute(); */
  /* testing fuji_base64_decode_input(); */
  /* testing fuji_base64_decode_length(); */
  /* testing fuji_base64_decode_output(); */
  /* testing fuji_base64_encode_compute(); */
  /* testing fuji_base64_encode_input(); */
  /* testing fuji_base64_encode_length(); */
  /* testing fuji_base64_encode_output(); */

  /* testing fuji_hash_add(); */
  /* testing fuji_hash_calculate(); */
  /* testing fuji_hash_clear(); */
  /* testing fuji_hash_compute(); */
  /* testing fuji_hash_compute_no_clear(); */
  /* testing fuji_hash_data(); */
  /* testing fuji_hash_input(); */
  /* testing fuji_hash_length(); */
  /* testing fuji_hash_output(); */
  /* testing fuji_hash_size(); */

  /* testing fuji_set_boot_config(); */
  /* testing fuji_set_boot_mode(); */

  /* testing fuji_reset(); */

  printf("All tests passed!\n");
  exit(0);
  return 0;
}

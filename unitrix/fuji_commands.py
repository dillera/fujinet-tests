from enum import Enum

class FUJICMD(Enum):
  RESET                      = 0xFF
  GET_SSID                   = 0xFE
  SCAN_NETWORKS              = 0xFD
  GET_SCAN_RESULT            = 0xFC
  SET_SSID                   = 0xFB
  GET_WIFISTATUS             = 0xFA
  MOUNT_HOST                 = 0xF9
  MOUNT_IMAGE                = 0xF8
  OPEN_DIRECTORY             = 0xF7
  READ_DIR_ENTRY             = 0xF6
  CLOSE_DIRECTORY            = 0xF5
  READ_HOST_SLOTS            = 0xF4
  WRITE_HOST_SLOTS           = 0xF3
  READ_DEVICE_SLOTS          = 0xF2
  WRITE_DEVICE_SLOTS         = 0xF1
  ENABLE_UDPSTREAM           = 0xF0
  SET_BAUDRATE               = 0xEB
  GET_WIFI_ENABLED           = 0xEA
  UNMOUNT_IMAGE              = 0xE9
  GET_ADAPTERCONFIG          = 0xE8
  NEW_DISK                   = 0xE7
  UNMOUNT_HOST               = 0xE6
  GET_DIRECTORY_POSITION     = 0xE5
  SET_DIRECTORY_POSITION     = 0xE4
  SET_HSIO_INDEX             = 0xE3
  SET_DEVICE_FULLPATH        = 0xE2
  SET_HOST_PREFIX            = 0xE1
  GET_HOST_PREFIX            = 0xE0
  SET_SIO_EXTERNAL_CLOCK     = 0xDF
  WRITE_APPKEY               = 0xDE
  READ_APPKEY                = 0xDD
  OPEN_APPKEY                = 0xDC
  CLOSE_APPKEY               = 0xDB
  GET_DEVICE_FULLPATH        = 0xDA
  CONFIG_BOOT                = 0xD9
  COPY_FILE                  = 0xD8
  MOUNT_ALL                  = 0xD7
  SET_BOOT_MODE              = 0xD6
  ENABLE_DEVICE              = 0xD5
  DISABLE_DEVICE             = 0xD4
  RANDOM_NUMBER              = 0xD3
  DEVICE_ENABLE_STATUS       = 0xD1
  BASE64_ENCODE_INPUT        = 0xD0
  BASE64_ENCODE_COMPUTE      = 0xCF
  BASE64_ENCODE_LENGTH       = 0xCE
  BASE64_ENCODE_OUTPUT       = 0xCD
  BASE64_DECODE_INPUT        = 0xCC
  BASE64_DECODE_COMPUTE      = 0xCB
  BASE64_DECODE_LENGTH       = 0xCA
  BASE64_DECODE_OUTPUT       = 0xC9
  HASH_INPUT                 = 0xC8
  HASH_COMPUTE               = 0xC7
  HASH_LENGTH                = 0xC6
  HASH_OUTPUT                = 0xC5
  GET_ADAPTERCONFIG_EXTENDED = 0xC4
  HASH_COMPUTE_NO_CLEAR      = 0xC3
  HASH_CLEAR                 = 0xC2
  GET_HEAP                   = 0xC1
  QRCODE_OUTPUT              = 0xBF
  QRCODE_LENGTH              = 0xBE
  QRCODE_ENCODE              = 0xBD
  QRCODE_INPUT               = 0xBC
  GET_DEVICE8_FULLPATH       = 0xA7
  GET_DEVICE7_FULLPATH       = 0xA6
  GET_DEVICE6_FULLPATH       = 0xA5
  GET_DEVICE5_FULLPATH       = 0xA4
  GET_DEVICE4_FULLPATH       = 0xA3
  GET_DEVICE3_FULLPATH       = 0xA2
  GET_DEVICE2_FULLPATH       = 0xA1
  GET_DEVICE1_FULLPATH       = 0xA0
  WRITE                      = 0x57
  STATUS                     = 0x53
  READ                       = 0x52
  OPEN                       = 0x4F
  CLOSE                      = 0x43
  HSIO_INDEX                 = 0x3F
  SEND_ERROR                 = 0x02
  SEND_RESPONSE              = 0x01
  DEVICE_READY               = 0x00

  # Clock commands
  GET_TIME_SOS               = ord('S')
  GET_TIME_ISO               = ord('I')

# Argument types:
#    i8: 8-bit integer
#   i16: 16-bit integer
#    u8: 8-bit unsigned
#   u16: 16-bit unsigned
#     s: variable length string/bytes
#    s8: variable length string/bytes with 8-bit length
#   s16: variable length string/bytes with 16-bit length
#  fNNN: fixed length string/bytes which will be padded to length

FujiCommandArgs = {
  FUJICMD.RESET: {},
  FUJICMD.GET_SSID: {'reply': ["data:s8", ]},
  FUJICMD.SCAN_NETWORKS: {'reply': ["count:u8", ]},
  FUJICMD.GET_SCAN_RESULT: {'args': ["index:u8", ], 'reply': ["data:s8", ]},
  FUJICMD.SET_SSID: {},
  FUJICMD.GET_WIFISTATUS: {'reply': ["status:u8", ]},
  FUJICMD.MOUNT_HOST: {'args': ["host_slot:u8", ]},
  FUJICMD.MOUNT_IMAGE: {'args': ["device_slot:u8", "mode:u8"]},
  FUJICMD.OPEN_DIRECTORY: {'args': ["host_slot:u8", "path:s8"]},
  FUJICMD.READ_DIR_ENTRY: {'args': ["maxlen:u8", "addtl:u8"], 'reply': ["entry:s7", ]},
  FUJICMD.CLOSE_DIRECTORY: {},
  FUJICMD.READ_HOST_SLOTS: {'reply': ["datas:f256", ]},
  FUJICMD.WRITE_HOST_SLOTS: {'args': ["config:f256", ]},
  FUJICMD.READ_DEVICE_SLOTS: {'reply': ["data:f304", ]},
  FUJICMD.WRITE_DEVICE_SLOTS: {'args': ["config:f304", ]},
  FUJICMD.ENABLE_UDPSTREAM: {},
  FUJICMD.SET_BAUDRATE: {},
  FUJICMD.GET_WIFI_ENABLED: {'reply': ["data:s8", ]},
  FUJICMD.UNMOUNT_IMAGE: {'args': ["device_slot:u8", ]},
  FUJICMD.GET_ADAPTERCONFIG: {'reply': ["data:s8", ]},
  FUJICMD.NEW_DISK: {'args': ["size:u8", "host_slot:u8", "device_slot:u8", "filename:s8"]},
  FUJICMD.UNMOUNT_HOST: {'args': ["host_slot:u8", ]},
  FUJICMD.GET_DIRECTORY_POSITION: {'reply': ["position:u16", ]},
  FUJICMD.SET_DIRECTORY_POSITION: {'args': ["position:u16", ]},
  FUJICMD.SET_HSIO_INDEX: {},
  FUJICMD.SET_DEVICE_FULLPATH: {'args': ["device_slot:u8", "host_slot:u8", "mode:u8", "filename:f256"]},
  FUJICMD.SET_HOST_PREFIX: {'args': ["host_slot:u8", "prefix:f256"]},
  FUJICMD.GET_HOST_PREFIX: {'args': ["host_slot:u8", ], 'reply': ["prefix:f256", ]},
  FUJICMD.SET_SIO_EXTERNAL_CLOCK: {},
  FUJICMD.WRITE_APPKEY: {'args': ["data:s16"]},
  FUJICMD.READ_APPKEY: {'reply': ["data:s8", ]},
  FUJICMD.OPEN_APPKEY: {'args': ["creator:u16", "app:u8", "key:u8", "mode:i8", "reserved:u8"]},
  FUJICMD.CLOSE_APPKEY: {},
  FUJICMD.GET_DEVICE_FULLPATH: {'args': ["device_slot:u8", ], 'reply': ["path:f256", ]},
  FUJICMD.CONFIG_BOOT: {'args': ["enable:u8", ]},
  FUJICMD.COPY_FILE: {'args': ["source_slot:u8", "dest_slot:u8", "copy_spec:s8"]},
  FUJICMD.MOUNT_ALL: {},
  FUJICMD.SET_BOOT_MODE: {'args': ["mode:u8", ]},
  FUJICMD.ENABLE_DEVICE: {'args': ["device:u8", ]},
  FUJICMD.DISABLE_DEVICE: {'args': ["device:u8", ]},
  FUJICMD.RANDOM_NUMBER: {},
  FUJICMD.DEVICE_ENABLE_STATUS: {},
  FUJICMD.BASE64_ENCODE_INPUT: {'args': ["length:u16", "data:s8"]},
  FUJICMD.BASE64_ENCODE_COMPUTE: {},
  FUJICMD.BASE64_ENCODE_LENGTH: {},
  FUJICMD.BASE64_ENCODE_OUTPUT: {'args': ["length:u16", ]},
  FUJICMD.BASE64_DECODE_INPUT: {'args': ["length:u16", "data:s8"]},
  FUJICMD.BASE64_DECODE_COMPUTE: {},
  FUJICMD.BASE64_DECODE_LENGTH: {},
  FUJICMD.BASE64_DECODE_OUTPUT: {'args': ["length:u16", ]},
  FUJICMD.HASH_INPUT: {'args': ["data:s16"]},
  FUJICMD.HASH_COMPUTE: {'args': ["algorithm:u8", ]},
  FUJICMD.HASH_LENGTH: {'args': ["as_hex:b", ]},
  FUJICMD.HASH_OUTPUT: {'args': ["as_hex:b", ]},
  FUJICMD.GET_ADAPTERCONFIG_EXTENDED: {'reply': ["data:s8", ]},
  FUJICMD.HASH_COMPUTE_NO_CLEAR: {},
  FUJICMD.HASH_CLEAR: {},
  FUJICMD.GET_HEAP: {},
  FUJICMD.QRCODE_OUTPUT: {'args': ["mode:u8", ]},
  FUJICMD.QRCODE_LENGTH: {},
  FUJICMD.QRCODE_ENCODE: {'args': ["version:u8", "ecc_mode:u8", "shorten:u8"]},
  FUJICMD.QRCODE_INPUT: {'args': ["length:u16", "data:s8"]},
  FUJICMD.GET_DEVICE8_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.GET_DEVICE7_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.GET_DEVICE6_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.GET_DEVICE5_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.GET_DEVICE4_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.GET_DEVICE3_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.GET_DEVICE2_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.GET_DEVICE1_FULLPATH: {'reply': ["path:f256", ]},
  FUJICMD.WRITE: {'args': ["handle:u8", "data:s"]},
  FUJICMD.STATUS: {'reply': ["status:u32", ]},
  FUJICMD.READ: {'args': ["handle:u8"]},
  FUJICMD.OPEN: {'args': ["mode:u8", "data:s"]},
  FUJICMD.CLOSE: {},
  FUJICMD.HSIO_INDEX: {},
  FUJICMD.SEND_ERROR: {},
  FUJICMD.SEND_RESPONSE: {},
  FUJICMD.DEVICE_READY: {},

  # Clock commands
  FUJICMD.GET_TIME_SOS: {},
  FUJICMD.GET_TIME_ISO: {},
}

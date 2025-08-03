#!/usr/bin/env python3
import argparse
import sys
import requests
import time
import socket
from fuji_test import *
from serial_monitor import SerialMonitor

SERVER_PORT = 7357

def build_argparser():
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("serial", help="serial port")
  parser.add_argument("--baud", type=int, default=460800, help="baud rate")
  return parser

FUJI_TESTS = [
  FujiTest(command=FUJICMD.SET_HOST_PREFIX, aux=[1, ], data="/test", warnOnly=True),
  FujiTest(command=FUJICMD.GET_HOST_PREFIX, aux=[1, ], warnOnly=True,
              replyLength=MAX_FILENAME_LEN, replyType=RType.NULTermString, expected=b"/test"),
  FujiTest(command=FUJICMD.HASH_INPUT, data="testing"),
  FujiTest(command=FUJICMD.HASH_COMPUTE, aux=[1, ]),
  FujiTest(command=FUJICMD.HASH_LENGTH, aux=[1, ], replyLength=1),
  FujiTest(command=FUJICMD.HASH_OUTPUT, aux=[1, ], replyLength=40),

  # The reply values of these commands will vary depending on config
  FujiTest(command=FUJICMD.READ_HOST_SLOTS, replyLength=256),
  FujiTest(command=FUJICMD.READ_DEVICE_SLOTS, replyLength=304),
  FujiTest(command=FUJICMD.GET_DEVICE1_FULLPATH, replyLength=256),

  # This appears to be a legacy command no longer supported?
  FujiTest(command=FUJICMD.GET_DEVICE_FULLPATH, aux=[1, ], replyLength=256, warnOnly=True),

  # Clock tests
  FujiTest(device=FujiDevice.APETIME, command=FUJICMD.GET_TIME_ISO,
           replyLength=25, replyType=RType.NULTermString)
]

ISSUE_910_TESTS = [
  FujiTest(command=FUJICMD.MOUNT_IMAGE, aux=[1, 0]),
]

def main():
  args = build_argparser().parse_args()

  guruWatch = SerialMonitor(args.serial, args.baud)
  guruWatch.start()

  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", SERVER_PORT))
    server.listen(1)
    print(f"Listening on {server.getsockname()[0]}:{server.getsockname()[1]}...")

    conn, addr = server.accept()
    with conn:
      print(f"Connected by {addr}")

      # FIXME - get FujiNet firmware version and type of machine running tests

      # FIXME - load tests to run from JSON file
      tests_to_run = FUJI_TESTS
      #tests_to_run = ISSUE_910_TESTS

      # Loop through fuji commands
      for test in tests_to_run:
        if test.runTest(conn, guruWatch) >= TestResult.FAIL:
          return 1

  # FIXME - display a summary of all tests and their results

  server.close()
  return

if __name__ == "__main__":
  exit(main() or 0)

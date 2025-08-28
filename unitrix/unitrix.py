#!/usr/bin/env python3
import argparse
import sys
import requests
import time
import socket
from serial_monitor import SerialMonitor
from fuji_test import *
from file_test import FileTest
from disk_test import DiskTest

SERVER_PORT = 7357
MOUNT_READ = 0
MOUNT_RDWR = 2

def build_argparser():
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("serial", help="serial port")
  parser.add_argument("--baud", type=int, default=460800, help="baud rate")
  return parser

FUJI_TESTS = [
  FujiTest(command=FUJICMD.SET_HOST_PREFIX, host_slot=1, prefix="/test", warnOnly=True),
  FujiTest(command=FUJICMD.GET_HOST_PREFIX, host_slot=1, warnOnly=True,
              replyLength=MAX_FILENAME_LEN, replyType=RType.NULTermString, expected=b"/test"),
  FujiTest(command=FUJICMD.HASH_INPUT, data="testing"),
  FujiTest(command=FUJICMD.HASH_COMPUTE, algorithm=1),
  FujiTest(command=FUJICMD.HASH_LENGTH, as_hex=True, replyLength=1),
  FujiTest(command=FUJICMD.HASH_OUTPUT, as_hex=True, replyLength=40),

  # The reply values of these commands will vary depending on config
  FujiTest(command=FUJICMD.READ_HOST_SLOTS, replyLength=256),
  FujiTest(command=FUJICMD.READ_DEVICE_SLOTS, replyLength=304),
  FujiTest(command=FUJICMD.GET_DEVICE1_FULLPATH, replyLength=256),

  # This appears to be a legacy command no longer supported?
  FujiTest(command=FUJICMD.GET_DEVICE_FULLPATH, device_slot=1, replyLength=256, warnOnly=True),

  # Clock tests
  FujiTest(device=FujiDevice.APETIME, command=FUJICMD.GET_TIME_ISO,
           replyLength=25, replyType=RType.NULTermString),

  # Mount Image should fail if host is not mounted first:
  FujiTest(command=FUJICMD.MOUNT_IMAGE, device_slot=1, mode=MOUNT_READ, errorExpected=True),
]

A2_PREFIX = "/FNTEST.APPLE2"
A2_HOST = 4
A2_DRIVE = 0
ISSUE_910_TESTS = [
  FujiTest(command=FUJICMD.MOUNT_HOST, host_slot=A2_HOST),
  FujiTest(command=FUJICMD.MOUNT_IMAGE, device_slot=A2_DRIVE, mode=MOUNT_RDWR),
  FileTest(OPEN_WRITE, A2_PREFIX + "/TESTDATA", "DOES THIS WORK"),
  FileTest(OPEN_READ, A2_PREFIX + "/TESTDATA", "DOES THIS WORK".encode("UTF-8")),
]

COCO_DIR_TESTS = [
  FujiTest(command=FUJICMD.MOUNT_HOST, host_slot=0),
  FujiTest(command=FUJICMD.OPEN_DIRECTORY, host_slot=0, path="/"),
  FujiTest(command=FUJICMD.READ_DIR_ENTRY, maxlen=32, addtl=0, replyLength=32),
  FujiTest(command=FUJICMD.CLOSE_DIRECTORY),
]

DOS33_TESTS = [
  DiskTest(MOUNT_READ, "apps.irata.online", "/APPLE_II/Testing/dos33.woz", 2),
]

def print_results(tests):
  print("Test results:")

  width = max([len(f"{test.command}") for test in tests])
  for test in tests:
    print(f"{test.command:<{width}} {test.result.name}")

  return

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
      #tests_to_run = FUJI_TESTS
      #tests_to_run = ISSUE_910_TESTS
      #tests_to_run = COCO_DIR_TESTS
      tests_to_run = DOS33_TESTS

      # Loop through fuji commands
      for test in tests_to_run:
        result = test.runTest(conn, guruWatch)
        test.result = result
        if result >= TestResult.FAIL:
          break

    # Sleep to allow capturing of any backtraces
    time.sleep(1)
    print()

  print_results(tests_to_run)

  server.close()
  return

if __name__ == "__main__":
  exit(main() or 0)

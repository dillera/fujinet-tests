#!/usr/bin/env python3
import argparse
import os, sys
import requests
import time
import socket
import json
from serial_monitor import SerialMonitor
from fuji_test import *
from file_test import FileTest
from disk_test import MountTest
from lwm import LWM

SERVER_PORT = 7357
MOUNT_READ = 0
MOUNT_RDWR = 2

def build_argparser():
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("serial", help="serial port or LWM binary")
  parser.add_argument(metavar="tests.json", dest="tests_json", nargs="+",
                      help="json file describing tests to run")
  parser.add_argument("--baud", type=int, default=460800, help="baud rate")
  parser.add_argument("--fnconfig", help="path to fnconfig.ini for use with LWM")
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
  MountTest(MOUNT_READ, "SD", "/NEWDO1.DO", 4),
]

def print_results(test_series):
  print("Test results:")

  for tests in test_series:
    names = [f"{getattr(test, 'command', test.__class__.__name__)}" for test in tests]
    width = max([len(f"{n}") for n in names])
    for idx, test in enumerate(tests):
      status = getattr(test, 'result', None)
      if status is None:
        status = 'NOT RUN'
      else:
        status = status.name
      print(f"{names[idx]:<{width}} {status}")

  return

def get_ip():
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  s.settimeout(0)
  try:
    # doesn't even have to be reachable
    s.connect(('10.254.254.254', 1))
    IP = s.getsockname()[0]
  except Exception:
    IP = '127.0.0.1'
  finally:
    s.close()
  return IP

def loadTests(path):
  with open(path, "r") as f:
    test_list = json.load(f)

  tests = []
  for test in test_list:
    test_type = test.pop('test', "Fuji")
    test_class = globals()[test_type + "Test"]
    tests.append(test_class(**test))

  return tests

def main():
  args = build_argparser().parse_args()

  if os.path.isfile(args.serial):
    guruWatch = LWM(args.serial, args.fnconfig)
  else:
    guruWatch = SerialMonitor(args.serial, args.baud)
  guruWatch.start()

  test_series = [loadTests(path) for path in args.tests_json]

  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", SERVER_PORT))
    server.listen(1)
    print(f"Listening on {get_ip()}:{server.getsockname()[1]}...")

    conn, addr = server.accept()
    with conn:
      print(f"Connection from {addr}")

      ace = conn.recv(240)
      print("Adapter config:");
      print(ace)

      # FIXME - get FujiNet firmware version and type of machine running tests

      for test_group in test_series:
        # Loop through fuji commands
        for test in test_group:
          result = test.runTest(conn, guruWatch)
          test.result = result
          if result >= TestResult.FAIL:
            break

    # Sleep to allow capturing of any backtraces
    time.sleep(1)
    print()

  print_results(test_series)

  server.close()
  return

if __name__ == "__main__":
  exit(main() or 0)

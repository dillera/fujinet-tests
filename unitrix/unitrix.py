#!/usr/bin/env python3
import argparse
import sys
import requests
import time
import socket
from fuji_test import FUJI_TESTS, TestResult
from serial_monitor import SerialMonitor

SERVER_PORT = 7357

def build_argparser():
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("serial", help="serial port")
  parser.add_argument("--baud", type=int, default=460800, help="baud rate")
  return parser

def main():
  args = build_argparser().parse_args()

  guruWatch = SerialMonitor(args.serial, args.baud)
  guruWatch.start()

  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", SERVER_PORT))
    server.listen(1)
    print(f'Listening on {server.getsockname()[0]}:{server.getsockname()[1]}...')

    conn, addr = server.accept()
    with conn:
      print(f'Connected by {addr}')

      # FIXME - get FujiNet firmware version and type of machine running tests

      # Loop through fuji commands
      for test in FUJI_TESTS:
        if test.runTest(conn, guruWatch) >= TestResult.FAIL:
          return 1

  # FIXME - display a summary of all tests and their results

  server.close()
  return

if __name__ == "__main__":
  exit(main() or 0)

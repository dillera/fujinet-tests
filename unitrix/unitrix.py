#!/usr/bin/env python3
import argparse
import sys
import threading
import requests
import serial
import time
import subprocess
import socket
from fuji_commands import FUJI_COMMANDS

TRIGGER_STRING = 'Guru Meditation Error'
SERVER_PORT = 7357

def build_argparser():
  parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument("serial", help="serial port")
  parser.add_argument("--baud", type=int, default=460800, help="baud rate")
  return parser

# Shared flags
trigger_found = False
fetch_failed = False

def monitor_serial(port, speed, trigger):
  global trigger_found
  try:
    with serial.Serial(port, speed, timeout=0.5) as ser:
      while not trigger_found and not fetch_failed:
        line = ser.readline().decode(errors='ignore')
        if line:
          print(f"CONSOLE: {line}", end="", flush=True)
        if trigger in line:
          print(f"Detected: {trigger}", file=sys.stderr)
          trigger_found = True
  except Exception as e:
    print(f"Serial error: {e}", file=sys.stderr)
    trigger_found = True

def main():
  args = build_argparser().parse_args()

  serial_thread = threading.Thread(
    target=monitor_serial,
    args=(args.serial, args.baud, TRIGGER_STRING),
    daemon=True
  )
  serial_thread.start()

  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", SERVER_PORT))
    server.listen(1)
    print(f'Listening on {server.getsockname()[0]}:{server.getsockname()[1]}...')

    conn, addr = server.accept()
    with conn:
      print(f'Connected by {addr}')

      # Loop through fuji commands
      for command in FUJI_COMMANDS:
        print("Testing command", command.command)
        conn.sendall(command.header())
        if command.data:
          conn.sendall(command.data.encode("utf-8"))

        if trigger_found:
          print(f"GURU ERROR test of {command.command} FAIL")
          return 1

        err = True
        data = conn.recv(1)
        if data:
          err = data[0]
        print("Result:", err)

        if err:
          if not command.warnOnly:
            print(f"Test of {command.command} FAIL")
            return 1
          print(f"WARNING test of {command.command} did not succeed")
        else:
          if command.replyLength:
            reply = conn.recv(4096)
            print(f"Reply received: {reply}")
            if command.expected and reply != command.expected:
              print("Data mismatch.\n"
                    f"  Expected \"{command.reply}\"\n"
                    f"  Received \"{reply}\"\n")
              return 1

  # FIXME - display a summary of all tests and their results

  server.close()
  return

if __name__ == "__main__":
  exit(main() or 0)

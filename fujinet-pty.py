#!/usr/bin/env python3
import os
import pty
import subprocess
import select
import sys
import fcntl
import tty, termios
import signal

def run_emulator_and_mirror(emulator_cmd):
  parent_pty, child_pty = pty.openpty()
  e_parent_pty, e_child_pty = pty.openpty()
  child_name = os.ttyname(child_pty)
  print(f"Test should open this device as serial port: {child_name}", file=sys.stderr)

  proc = subprocess.Popen(
    emulator_cmd,
    stdin=e_child_pty,
    stdout=e_child_pty,
    stderr=subprocess.STDOUT,
    close_fds=True,
    text=False,  # binary mode for raw bytes
  )
  print(f"Emulator PID is: {proc.pid}")

  os.close(e_child_pty)
  os.close(child_pty)

  flags = fcntl.fcntl(parent_pty, fcntl.F_GETFL)
  fcntl.fcntl(parent_pty, fcntl.F_SETFL, flags | os.O_NONBLOCK)

  fd = sys.stdin.fileno()
  old_settings = termios.tcgetattr(fd) # Save current terminal settings
  tty.setraw(fd)

  try:
    while True:
      rlist, _, _ = select.select([sys.stdin, e_parent_pty], [], [])
      if e_parent_pty in rlist:
        data = os.read(e_parent_pty, 1024)
        if not data:
          break
        # Mirror to wrapper's stdout
        os.write(sys.stdout.fileno(), data)
        # Mirror to pty parent FD
        try:
          os.write(parent_pty, data)
        except BlockingIOError:
          pass
      else:
        key = os.read(sys.stdin.fileno(), 1)
        if key[0] == 3:
          proc.send_signal(signal.SIGINT)
        else:
          os.write(e_parent_pty, key)
  finally:
    os.close(e_parent_pty)
    os.close(parent_pty)
    proc.wait()
    termios.tcsetattr(fd, termios.TCSADRAIN, old_settings) # Restore original settings

if __name__ == "__main__":
  run_emulator_and_mirror(["gdb", "-ex", "run", "--args", "./fujinet", "-c", "../../fnconfig.ini"])

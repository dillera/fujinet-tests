import os
import pty
import subprocess
import select
import sys
import fcntl
import tty, termios
import signal
import threading

class LWM:
  def __init__(self, lwmPath, configPath=None):
    self.triggerFound = False
    self.parent_pty, self.child_pty = pty.openpty()
    self.e_parent_pty, self.e_child_pty = pty.openpty()
    self.serial = os.ttyname(self.child_pty)
    #print(f"Test should open this device as serial port: {self.serial}", file=sys.stderr)

    lwmPath = os.path.abspath(lwmPath)
    lwmDir = os.path.dirname(lwmPath)
    os.chdir(lwmDir)
    self.cmd = ["gdb", "--fullname", "-ex", "run", "--args", lwmPath]
    if configPath:
      self.cmd.extend(["-c", configPath])

    return

  def start(self):
    self.thread = threading.Thread(target=self.run, daemon=True)
    self.thread.start()
    return

  def run(self):
    proc = subprocess.Popen(
      self.cmd,
      stdin=self.e_child_pty,
      stdout=self.e_child_pty,
      stderr=subprocess.STDOUT,
      close_fds=True,
      text=False,  # binary mode for raw bytes
    )
    print(f"Emulator PID is: {proc.pid}")

    os.close(self.e_child_pty)
    os.close(self.child_pty)

    flags = fcntl.fcntl(self.parent_pty, fcntl.F_GETFL)
    fcntl.fcntl(self.parent_pty, fcntl.F_SETFL, flags | os.O_NONBLOCK)

    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd) # Save current terminal settings
    tty.setraw(fd)

    try:
      while True:
        rlist, _, _ = select.select([sys.stdin, self.e_parent_pty], [], [])
        if self.e_parent_pty in rlist:
          try:
            data = os.read(self.e_parent_pty, 1024)
          except OSError:
            break
          if not data:
            break
          # Mirror to wrapper's stdout
          os.write(sys.stdout.fileno(), data)
          # Mirror to pty parent FD
          try:
            os.write(self.parent_pty, data)
          except BlockingIOError:
            pass
        else:
          key = os.read(sys.stdin.fileno(), 1)
          if key[0] == 3:
            proc.send_signal(signal.SIGINT)
          else:
            os.write(self.e_parent_pty, key)
    finally:
      os.close(self.e_parent_pty)
      os.close(self.parent_pty)
      proc.wait()
      termios.tcsetattr(fd, termios.TCSADRAIN, old_settings) # Restore original settings

    return

  def clearBuffer(self):
    return

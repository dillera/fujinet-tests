import threading
import serial
import sys
import socket

GURU_ERROR = 'Guru Meditation Error'

class SerialMonitor:
  def __init__(self, port, speed, trigger=GURU_ERROR):
    self.triggerFound = False
    self.wakeupRecv, self.wakeupSend = socket.socketpair()
    if port == "-":
      return
    self.port = serial.Serial(port, speed, timeout=0.5)
    self.trigger = trigger
    self.buffer = []
    return

  def monitor(self):
    try:
      while True: # not self.triggerFound:
        line = self.port.readline().decode(errors='ignore')
        if line:
          self.buffer.append(line)
          #print(f"CONSOLE: {line}", end="", flush=True)
        if self.trigger in line:
          print(f"Detected: {self.trigger}", file=sys.stderr)
          # FIXME - collect Backtrace
          self.triggerFound = True
          self.wakeupSend.send(b'E')
    except Exception as e:
      print(f"Serial error: {e}", file=sys.stderr)
      self.triggerFound = True
    return

  def clearBuffer(self):
    self.buffer = []
    return

  def start(self):
    if not hasattr(self, 'port'):
      return
    self.thread = threading.Thread(target=self.monitor, daemon=True)
    self.thread.start()
    return

  @property
  def errorSocket(self):
    return self.wakeupRecv

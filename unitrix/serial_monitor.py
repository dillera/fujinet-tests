import threading
import serial
import sys

GURU_ERROR = 'Guru Meditation Error'

class SerialMonitor:
  def __init__(self, port, speed, trigger=GURU_ERROR):
    self.triggerFound = False
    self.serial = serial.Serial(port, speed, timeout=0.5)
    self.trigger = trigger
    self.buffer = []
    return

  def monitor(self):
    try:
      while not self.triggerFound:
        line = self.serial.readline().decode(errors='ignore')
        if line:
          self.buffer.append(line)
          print(f"CONSOLE: {line}", end="", flush=True)
        if self.trigger in line:
          print(f"Detected: {trigger}", file=sys.stderr)
          self.triggerFound = True
    except Exception as e:
      print(f"Serial error: {e}", file=sys.stderr)
      self.triggerFound = True
    return

  def clearBuffer(self):
    self.buffer = []
    return

  def start(self):
    self.thread = threading.Thread(target=self.monitor, daemon=True)
    self.thread.start()
    return

from dataclasses import dataclass
import struct
from enum import Enum, IntEnum
from fuji_commands import FUJICMD

FLAG_WARN = 0x10

MAX_FILENAME_LEN = 256

class RType(Enum):
  NULTermString = 1

class TestResult(IntEnum):
  PASS       = 0
  WARNING    = 1
  FAIL       = 2
  GURU_ERROR = 3

class ErrCode(Enum):
  ReceiveError = -1
  DataMismatch = -2
  GuruError    = -2

class FujiDevice(Enum):
  DISK         = 0x31
  DISK_LAST    = 0x3F
  PRINTER      = 0x40
  PRINTER_LAST = 0x43
  VOICE        = 0x43
  APETIME      = 0x45
  SERIAL       = 0x50
  SERIAL_LAST  = 0x53
  THEFUJI      = 0x70
  NETWORK      = 0x71
  NETWORK_LAST = 0x78
  MIDI         = 0x99
  CPM          = 0x5A

@dataclass
class FujiTest:
  command: int
  device: int = FujiDevice.THEFUJI
  aux: list = None
  data: str = None
  replyLength: int = 0
  replyType: RType = None
  expected: str = None
  warnOnly: bool = False

  def header(self) -> bytes:
    if not self.aux:
      self.aux = []
    auxlen = len(self.aux)
    flags = (1 << auxlen) - 1
    if self.warnOnly:
      flags |= FLAG_WARN
    aux_data = self.aux
    if not aux_data:
      aux_data = []
    aux_data = (aux_data + [0, 0, 0, 0])[:4]
    return struct.pack("<BB B BBBB HH", self.device.value, self.command.value,
                       flags, *aux_data,
                       0 if not self.data else len(self.data), self.replyLength)

  def runTest(self, conn, serial):
    serial.clearBuffer()

    print("Testing command", self.command)
    conn.sendall(self.header())
    if self.data:
      conn.sendall(self.data.encode("utf-8"))

    self.errcode = ErrCode.ReceiveError
    data = conn.recv(1)
    if data:
      self.errcode = data[0]
    print("Result:", self.errcode)

    if not self.errcode and self.replyLength:
      self.reply = conn.recv(4096)
      if self.replyType == RType.NULTermString:
        self.reply = self.reply.split(b"\x00")[0]
      print(f"Reply received: {self.reply}")
      if self.expected and not self.validate():
        print("Data mismatch.\n"
              f"  Expected \"{self.expected}\"\n"
              f"  Received \"{self.reply}\"\n")
        self.errcode = ErrCode.DataMismatch

    self.log = serial.buffer

    if serial.triggerFound:
      prefix = "### GURU ERROR ###"
      self.errcode = ErrCode.GuruError

    if self.errcode:
      prefix = "**FAIL**"
      if self.warnOnly:
        prefix = "WARNING"
      print(f"{prefix} test of {self.command} did not succeed")
      if self.errcode == ErrCode.GuruError:
        return TestResult.GuruError
      if self.warnOnly:
        return TestResult.WARNING
      return TestResult.FAIL

    return TestResult.PASS

  def validate(self):
    # FIXME - do we need to gather expected data from current FujiNet config?
    return self.reply == self.expected

from dataclasses import dataclass
import struct
from enum import Enum, IntEnum
import select
from fuji_commands import FUJICMD, FujiCommandArgs
from hexdump import hexdump

FLAG_WARN = 0x10

MAX_FILENAME_LEN = 256

OPEN_READ  = 0x40
OPEN_WRITE = 0x80

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
  GuruError    = -3

class FujiDevice(Enum):
  DISK         = 0x31
  DISK_LAST    = 0x3F
  PRINTER      = 0x40
  PRINTER_LAST = 0x43
  VOICE        = 0x43
  APETIME      = 0x45
  SERIAL       = 0x50
  SERIAL_LAST  = 0x53
  CPM          = 0x5A
  THEFUJI      = 0x70
  NETWORK      = 0x71
  NETWORK_LAST = 0x78
  MIDI         = 0x99
  FILE         = 0xAA

class FujiTest:
  def __init__(self, **kwargs):
    try:
      command = kwargs.pop('command')
      if isinstance(command, str):
        command = FUJICMD[command.upper()]
      self.command = FUJICMD(command)
    except (TypeError, ValueError):
      raise ValueError("Unknown command:", command)

    self.device = kwargs.pop('device', FujiDevice.THEFUJI)
    self.expected = kwargs.pop('expected', None)
    self.warnOnly = kwargs.pop('warnOnly', False)
    self.errorExpected = kwargs.pop('errorExpected', False)
    self.data = None

    self.replyLength = kwargs.pop('replyLength', 0)
    self.replyType = kwargs.pop('replyType', None)
    
    # get arg list for self.command and make sure all args are present
    # and of required type
    cmdArgs = FujiCommandArgs.get(self.command, None)
    if cmdArgs is None:
      raise ValueError("Unknown Fuji Command", self.command)

    reply = cmdArgs.get('reply', None)
    if reply is not None:
      self.replyLength = 0
      for field in reply:
        replyName, replyType = field.split(':')
        self.replyLength += self.byteLength(replyType)
        if replyType[0] == 'n':
          self.replyType = RType.NULTermString
      
    self.assignArgs(cmdArgs, kwargs)
    return

  def assignArgs(self, cmdArgs, kwargs):
    reqArgs = cmdArgs.get('args', None)
    if not reqArgs:
      reqArgs = []

    self.aux = []
    for required in reqArgs:
      argName, argType = required.split(':')
      if argName not in kwargs:
        raise ValueError(f"{self.command} missing required argument: {argName}")

      value = kwargs.pop(argName)

      if argType[0] in ('i', 'u', 'b'):
        if argType[0] == 'b' and not isinstance(value, bool):
          raise ValueError(f"{argName} is not bool")
        elif not isinstance(value, int):
          raise ValueError(f"{argName} is not int", value)

        numBits = 1 if argType == 'b' else int(argType[1:])
        if not self.isValidInt(value, numBits, argType[0] == 'i'):
          raise ValueError(f"{argName} value {value}"
                           f" outside of allowed range {argRange.start}..{argRange.stop - 1}")

        self.aux.extend(self.intToBytes(value, numBits))

      elif argType[0] == 's':
        if not isinstance(value, (str, bytes, bytearray)):
          raise ValueError(f"{argName} is not str or bytes")

        if len(argType) > 1:
          numBits = int(argType[1:])
          if not self.isValidInt(len(value), numBits, False):
            raise ValueError(f"size of {argName} is longer than maximum {argMax - 1}")

          self.aux.extend(self.intToBytes(len(value), numBits))

        self.data = value

      elif argType[0] == 'f':
        if not isinstance(value, (str, bytes, bytearray)):
          raise ValueError(f"{argName} is not str or bytes")

        numBytes = int(argType[1:])
        if len(value) > numBytes:
          raise ValueError(f"size of {argName} exceeds maximum {numBytes}")

        self.data = (value + '\x00' * numBytes)[:numBytes]

    if len(kwargs):
      raise ValueError(f"{self.command} unrecognized args: {', '.join(kwargs.keys())}")

    return

  @staticmethod
  def isValidInt(value, numBits, isSigned):
    intMax = 1 << numBits
    if isSigned:
      intRange = range(intMax // -2, intMax // 2)
    else:
      intRange = range(0, intMax)
    return value in intRange

  @staticmethod
  def byteLength(argType):
    if argType[0] == 'b':
      return 1

    if argType[0] in ('i', 'u', 's'):
      numBits = int(argType[1:])
      return (numBits + 7) // 8

    if argType[0] in ('f', 'n'):
      return int(argType[1:])

    return None
    
  @staticmethod
  def intToBytes(value, numBits):
    bdata = []
    while numBits > 0:
      bdata.append(value & 0xFF)
      value >>= 8
      numBits -= 8
    return bdata

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

  def recv(self, count):
    # Allow the serial monitor to abort receiving if Guru Error is received
    readable, _, _ = select.select([self._conn, self._serial.errorSocket], [], [])
    if self._conn in readable:
      print("READING socket")
      try:
        return self._conn.recv(count)
      except ConnectionResetError:
        pass
    return None

  def sendall(self, data):
    self._conn.sendall(data)
    return

  def runTest(self, conn, serial):
    self._conn = conn
    self._serial = serial

    serial.clearBuffer()

    print(f"Testing command {self.command} {hex(self.command.value)}")
    hdata = self.header()
    hexdump(hdata)
    self.sendall(hdata)
    if self.data:
      data = self.data.encode("utf-8") if isinstance(self.data, str) else self.data
      self.sendall(data)

    self.errcode = ErrCode.ReceiveError
    data = self.recv(1)
    if data:
      self.errcode = data[0]
    print("Result:", self.errcode)

    if not self.errcode and self.replyLength:
      print("Expecting reply data");
      self.reply = self.recv(4096)

      if self.reply is not None:
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
        return TestResult.GURU_ERROR
      if self.warnOnly:
        return TestResult.WARNING
      return TestResult.FAIL

    return TestResult.PASS

  def validate(self):
    # FIXME - do we need to gather expected data from current FujiNet config?
    return self.reply == self.expected

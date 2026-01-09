from dataclasses import dataclass
import struct
from enum import Enum, IntEnum
import select
from fuji_commands import FUJICMD, FujiCommandArgs
from hexdump import hexdump

FLAG_WARN        = 0x10
FLAG_EXCEEDS_U8  = 0x04
FLAG_EXCEEDS_U16 = 0x02

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

    self.bigEndian=True

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

    self.assignArgs(cmdArgs.get('args', None), kwargs)
    return

  def _packInt(self, argName, argType, value):
    """Pack integer types: i8, i16, i32, u8, u16, u32, b"""
    if argType == 'b':
      if not isinstance(value, bool):
        raise ValueError(f"{argName} is not bool")
      numBits = 1
    else:
      if not isinstance(value, int):
        raise ValueError(f"{argName} is not int: {value}")
      numBits = int(argType[1:])

    is_signed = argType[0] == 'i'
    if not self.isValidInt(value, numBits, is_signed):
      raise ValueError(f"{argName} value {value} outside allowed range")

    return (self.intToBytes(value, numBits, self.bigEndian), 1)

  def _packString(self, argName, argType, value):
    """Pack string with length prefix: s8, s16, s32, or s (no prefix)"""
    if not isinstance(value, (str, bytes, bytearray)):
      raise ValueError(f"{argName} is not str or bytes")

    if len(argType) > 1:
      # String with length prefix
      numBits = int(argType[1:])
      if not self.isValidInt(len(value), numBits, False):
        raise ValueError(f"size of {argName} exceeds maximum")

      return (self.intToBytes(len(value), numBits, self.bigEndian), 1)

    # Plain string without length prefix
    return (bytearray(), 0)

  def _packFixed(self, argName, argType, value):
    """Pack fixed-length string: f8, f16, f32, etc."""
    if not isinstance(value, (str, bytes, bytearray)):
      raise ValueError(f"{argName} is not str or bytes")

    numBytes = int(argType[1:])
    if len(value) > numBytes:
      raise ValueError(f"size of {argName} exceeds maximum {numBytes}")

    # Pad to fixed length
    if isinstance(value, str):
      padded = (value + '\x00' * numBytes)[:numBytes]
    else:
      padded = (value + b'\x00' * numBytes)[:numBytes]

    return (padded, 0)

  def _packStruct(self, struct_spec, kwargs):
    """
    Pack struct type by extracting multiple named fields from kwargs.

    Args:
        struct_spec: String like "creator:u16,type:u8,reserved:u32"
        kwargs: Dictionary to pop values from

    Returns:
        (packedBytes, numArgs)
    """
    fields = struct_spec.split(',')
    packed = bytearray()
    total_args = 0

    for field in fields:
      field_name, field_type = field.split(':', 1)

      if field_name not in kwargs:
        raise ValueError(f"{self.command} missing required argument: {field_name}")

      value = kwargs.pop(field_name)
      field_packed, field_args = self.packValue(field_name, field_type, value)
      packed.extend(field_packed)
      total_args += field_args

    return (packed, total_args)

  def packValue(self, argName, argType, value):
    """
    Pack a single value according to its type specification.
    Returns (packedBytes, numArgs) where numArgs is count of variable-length items.

    Supported types:
    - i8, i16, i32, i64: signed integers
    - u8, u16, u32, u64: unsigned integers
    - b: boolean
    - s, s8, s16, s32: string with optional length prefix
    - f8, f16, f32: fixed-length string
    """
    if not argType:
      raise ValueError(f"{argName} has empty type specification")

    type_char = argType[0]

    if type_char in ('i', 'u', 'b'):
      return self._packInt(argName, argType, value)
    elif type_char == 's':
      return self._packString(argName, argType, value)
    elif type_char == 'f':
      return self._packFixed(argName, argType, value)

    raise ValueError(f"Unknown type specifier: {argType}")


  def assignArgs(self, reqArgs, kwargs):
    """
    Assign and pack arguments according to their type specifications.

    Args:
        reqArgs: List of specifications, either:
                 - "argname:type" for single arguments
                 - "{field1:type1,field2:type2,...}" for packed structs
        kwargs: Dictionary of argument values
    """
    self.numArgs = 0
    if not reqArgs:
      reqArgs = []

    self.aux = []
    self.data = None

    for required in reqArgs:
      # Check if this is a struct specification
      if required.startswith('{') and required.endswith('}'):
        struct_spec = required[1:-1]  # Strip the {}
        packed, _ = self._packStruct(struct_spec, kwargs)
        self.data = packed
        continue

      # Regular single argument
      if ':' not in required:
        raise ValueError(f"Invalid argument specification: {required}")

      argName, argType = required.split(':', 1)

      if argName not in kwargs:
        raise ValueError(f"{self.command} missing required argument: {argName}")

      value = kwargs.pop(argName)

      # Special handling for 's' and 'f' types that set self.data
      if argType[0] == 's':
        if len(argType) > 1:
          # String with length prefix: prefix goes in aux, data in self.data
          packed, num_args = self.packValue(argName, argType, value)
          self.aux.extend(packed)
          self.numArgs += num_args
        self.data = value
      elif argType[0] == 'f':
        # Fixed-length string goes in self.data
        packed, _ = self.packValue(argName, argType, value)
        self.data = packed
      else:
        # Everything else (integers) goes in aux
        packed, num_args = self.packValue(argName, argType, value)
        self.aux.extend(packed)
        self.numArgs += num_args

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
  def intToBytes(value, numBits, bigEndian):
    bdata = []
    while numBits > 0:
      bdata.append(value & 0xFF)
      value >>= 8
      numBits -= 8
    if bigEndian:
      bdata.reverse()
    return bdata

  def header(self) -> bytes:
    if not self.aux:
      self.aux = []
    auxlen = len(self.aux)
    flags = self.numArgs
    print("Num args:", self.numArgs, "Num bytes:", auxlen)
    if auxlen != self.numArgs:
      flags += FLAG_EXCEEDS_U8
      if self.numArgs == 1 and auxlen == 4:
        flags += FLAG_EXCEEDS_U16
      print("Flags:", flags)

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
    try:
      self._conn.sendall(data)
    except BrokenPipeError:
      pass
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
      hexdump(data)
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

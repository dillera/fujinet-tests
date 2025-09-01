from fuji_test import *
import struct
import select
from fuji_commands import FUJICMD

MAX_HOSTS = 8
MAX_HOSTNAME_LEN = 32
MAX_FILENAME_LEN = 256

class HostConfig:
  def __init__(self, packedData):
    self.slots = []
    for idx in range(MAX_HOSTS):
      url = packedData[idx * MAX_HOSTNAME_LEN:(idx + 1) * MAX_HOSTNAME_LEN]
      url = url.partition(b"\x00")[0].decode("UTF-8")
      self.slots.append(url)
    return

  def matchingSlot(self, url):
    for idx, sURL in enumerate(self.slots):
      if url.lower() == sURL.lower():
        return idx
    return None

  def firstBlank(self):
    return self.matchingSlot("")

  def setSlotURL(self, idx, url):
    self.slots[idx] = url
    return

class MountTest:
  def __init__(self, mode, hostURL, remotePath, driveSlot):
    self.mode = mode
    self.hostURL = hostURL
    self.remotePath = remotePath
    self.driveSlot = driveSlot
    return

  def runTest(self, conn, serial):
    subtest = FujiTest(command=FUJICMD.READ_HOST_SLOTS,
                       replyLength=MAX_HOSTS * MAX_HOSTNAME_LEN)
    result = subtest.runTest(conn, serial)
    if result != TestResult.PASS:
      return result

    hostConfig = HostConfig(subtest.reply)
    idx = hostConfig.matchingSlot(self.hostURL)
    if idx is None:
      idx = hostConfig.firstBlank()
      if idx is None:
        idx = MAX_HOSTS - 1
      hostConfigs.setSlotURL(idx, self.hostURL)

      subtest = FujiTest(command=FUJICMD.WRITE_HOST_SLOTS, config=hostConfig.data)
      result = subtest.runTest(conn, serial)
      if result != TestResult.PASS:
        return result

    subtest = FujiTest(command=FUJICMD.MOUNT_HOST, host_slot=idx)
    result = subtest.runTest(conn, serial)
    if result != TestResult.PASS:
      return result

    subtest = FujiTest(command=FUJICMD.SET_DEVICE_FULLPATH,
                       host_slot=idx, device_slot=self.driveSlot, mode=self.mode,
                       filename=self.remotePath)
    result = subtest.runTest(conn, serial)
    if result != TestResult.PASS:
      return result

    subtest = FujiTest(command=FUJICMD.MOUNT_IMAGE, device_slot=self.driveSlot,
                       mode=self.mode)
    result = subtest.runTest(conn, serial)
    if result != TestResult.PASS:
      return result

    return TestResult.PASS

# FIXME - add a way to get disk volumes as seen by computer. List
#         should include drive number, volume name/prefix

# Harness-level Apple II disk I/O test using remotecmd DISK pseudo-commands.
# These are not FUJICMD values; they are handled OS-side in remotecmd/diskcmd.c
# Command IDs (must match remotecmd/diskcmd.h):
#   OPEN=0x10, READ=0x11, WRITE=0x12, CLOSE=0x13, SEEK=0x14, STAT=0x15
DISKCMD_OPEN  = 0x10
DISKCMD_READ  = 0x11
DISKCMD_WRITE = 0x12
DISKCMD_CLOSE = 0x13
DISKCMD_SEEK  = 0x14
DISKCMD_STAT  = 0x15

DISK_OPEN_READ  = 0x40
DISK_OPEN_WRITE = 0x80

class DiskIOTest:
  def __init__(self, device_slot, filename, data):
    # device_slot: 0..(0x3F-0x31) maps to DISK devices 0x31..0x3F
    self.device_slot = device_slot
    self.filename = filename
    self.data = data if isinstance(data, (bytes, bytearray)) else data.encode('utf-8')
    return

  def _pack_header(self, device, command, flags, aux, data_len, reply_len):
    # aux is list of up to 4 bytes
    aux = (aux + [0, 0, 0, 0])[:4]
    return struct.pack("<BB B BBBB HH", device, command, flags, *aux, data_len, reply_len)

  def _send(self, conn, serial, header, payload=None):
    serial.clearBuffer()
    conn.sendall(header)
    if payload:
      conn.sendall(payload)
    # read 1-byte result
    readable, _, _ = select.select([conn, serial.errorSocket], [], [])
    if conn in readable:
      res = conn.recv(1)
      return res[0] if res else 0xFF
    return 0xFF

  def runTest(self, conn, serial):
    device = 0x31 + (self.device_slot & 0x0F)

    # OPEN for write
    hdr = self._pack_header(device, DISKCMD_OPEN, 0, [DISK_OPEN_WRITE], len(self.filename), 1)
    res = self._send(conn, serial, hdr, self.filename.encode('utf-8'))
    if res:
      return TestResult.FAIL
    readable, _, _ = select.select([conn, serial.errorSocket], [], [])
    if conn not in readable:
      return TestResult.FAIL
    handle = conn.recv(1)
    if not handle:
      return TestResult.FAIL
    handle = handle[0]

    # WRITE data
    hdr = self._pack_header(device, DISKCMD_WRITE, 0, [handle], len(self.data), 0)
    res = self._send(conn, serial, hdr, self.data)
    if res:
      return TestResult.FAIL

    # CLOSE
    hdr = self._pack_header(device, DISKCMD_CLOSE, 0, [handle], 0, 0)
    res = self._send(conn, serial, hdr)
    if res:
      return TestResult.FAIL

    # OPEN for read
    hdr = self._pack_header(device, DISKCMD_OPEN, 0, [DISK_OPEN_READ], len(self.filename), 1)
    res = self._send(conn, serial, hdr, self.filename.encode('utf-8'))
    if res:
      return TestResult.FAIL
    readable, _, _ = select.select([conn, serial.errorSocket], [], [])
    if conn not in readable:
      return TestResult.FAIL
    handle = conn.recv(1)
    if not handle:
      return TestResult.FAIL
    handle = handle[0]

    # READ back
    hdr = self._pack_header(device, DISKCMD_READ, 0, [handle], 0, len(self.data))
    res = self._send(conn, serial, hdr)
    if res:
      return TestResult.FAIL
    # Expect reply data
    readable, _, _ = select.select([conn, serial.errorSocket], [], [])
    if conn not in readable:
      return TestResult.FAIL
    reply = conn.recv(len(self.data))
    if reply != self.data:
      return TestResult.FAIL

    # CLOSE
    hdr = self._pack_header(device, DISKCMD_CLOSE, 0, [handle], 0, 0)
    res = self._send(conn, serial, hdr)
    if res:
      return TestResult.FAIL

    return TestResult.PASS

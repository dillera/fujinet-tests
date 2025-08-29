from fuji_test import *
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

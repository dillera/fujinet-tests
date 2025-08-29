from fuji_test import *
from fuji_commands import FUJICMD

class FileTest:
  def __init__(self, mode, filename, data):
    self.mode = mode
    self.filename = filename
    self.data = data
    return

  def runTest(self, conn, serial):
    subtest = FujiTest(device=FujiDevice.FILE, command=FUJICMD.OPEN, mode=self.mode,
           data=self.filename, replyLength=1)
    result = subtest.runTest(conn, serial)
    if result != TestResult.PASS:
      return result

    handle = subtest.reply[0]
    if self.mode == OPEN_WRITE:
      subtest = FujiTest(device=FujiDevice.FILE, command=FUJICMD.WRITE,
                         aux=[handle, ], data=self.data)
    else:
      subtest = FujiTest(device=FujiDevice.FILE, command=FUJICMD.READ,
                         aux=[handle, ], replyLength=len(self.data), expected=self.data)
    result = subtest.runTest(conn, serial)
    if result != TestResult.PASS:
      return result

    subtest = FujiTest(device=FujiDevice.FILE, command=FUJICMD.CLOSE, aux=[handle, ])
    result = subtest.runTest(conn, serial)
    return result

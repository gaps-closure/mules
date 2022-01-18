FE_BOILERPLATE='''#!/usr/bin/env python3

from queue     import Queue
from threading import Thread

class Block(Thread):
  def __init__(self, processor=None, nexthop=None):
    self.queue     = Queue()
    self.processor = processor
    self.nexthop   = nexthop

  def input(self, data): 
    self.queue.put(data)

  def start(self):
    while(True):
      try:
        idata = self.queue.get()
        odata = idata if self.processor is None else self.processor(idata) 
        self.nexthop(odata).input(odata)
      except:
        pass

'''

def python_backend(dagr_ir):
  s = FE_BOILERPLATE
  # generate blocks
  # connect blocks
  # populate blocks
  return s

FE_COMMON='''#!/usr/bin/env python3

import sys
import zmq
from time      import sleep
from queue     import Queue
from threading import Thread

ZMQ_IN_URI  = 'ipc:///tmp/dagr_in'
ZMQ_OUT_URI = 'ipc:///tmp/dagr_out'

def guard_eval(data, expr):
  return True

class Block(Thread):
  def __init__(self, blkname, engine):
    self.engine    = engine
    self.blkname   = blkname
    self.queue     = Queue()
    self.ftable    = []
    self.processor = None
    super().__init__(daemon=True)
  def populate(self, processor):     self.processor = processor
  def addNexthop(self, block, expr): self.ftable.append([block,expr])
  def input(self, data):             self.queue.put(data)
  def nexthop(self, data):           return [b for [b,e] in self.ftable if guard_eval(data,e)]
  def run(self):
    while True:
      try:
        idata = self.queue.get()
        odata = idata if self.processor is None else self.processor(idata) 
        for b in self.nexthop(odata):
          b.input(odata) # forward to first match only
          break
      except:
        pass

class ExitBlock(Block):
  def run(self):
    context = zmq.Context()
    socket  = context.socket(zmq.PUB)
    socket.bind(ZMQ_OUT_URI)
    while True:
      try:    socket.send(self.queue.get())
      except: pass

class EntryBlock(Block):
  def run(self):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.bind(ZMQ_IN_URI)
    socket.setsockopt_string(zmq.SUBSCRIBE, '')
    while True:
      try:    self.input(socket.recv())
      except: pass

class Engine:
  def __init__(self):
    self.blocks = {}
    self.add('entry')
    self.add('exit')
  def add(self, blkname=None): 
    if blkname not in self.blocks: 
      if blkname == 'entry':
        self.blocks[blkname] = EntryBlock(blkname, self) 
      elif blkname == 'exit':
        self.blocks[blkname] = ExitBlock(blkname, self) 
      else:
        self.blocks[blkname] = Block(blkname, self) 
  def connect(self, b1, b2, guard=None):
    if b1 in self.blocks and b2 in self.blocks: 
      self.blocks[b1].addNexthop(self.blocks[b2], guard)
  def populate(self, blkname, processor): 
    self.blocks[blkname].populate(processor)
  def start(self):
    for b in self.blocks.values(): b.start()
    while True: 
      try:                       sleep(1)
      except(KeyboardInterrupt): sys.exit(1)
      
if __name__ == '__main__':
  e = Engine()
'''

def transpile(expr):
  return ''

def python_backend(dagr_ir, output_file):
  with open(output_file, 'w') as f: 
    f.write(FE_COMMON)
    for b in dagr_ir.ruleblks:
      f.write("  e.add('%s')\n" % b) 
    # XXX: must transpile and populate blocks here
    for (b1,b2,g) in list(dagr_ir.pipeline.values())[0]:
      f.write("  e.connect('%s','%s','%s')\n" % (b1,b2,transpile(g)))
    f.write("  e.start()\n")

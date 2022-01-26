#!/usr/bin/env python3

import sys
import zmq
import xml.etree.ElementTree as     ET
from   time                  import sleep
from   queue                 import Queue
from   threading             import Thread

ZMQ_IN_URI  = 'ipc:///tmp/dagr_in'
ZMQ_OUT_URI = 'ipc:///tmp/dagr_out'

class Block(Thread):
  def __init__(self, blkname, engine):
    self.engine    = engine
    self.blkname   = blkname
    self.queue     = Queue()
    self.ftable    = []
    self.rules     = []
    super().__init__(daemon=True)
  def addNexthop(self, block, expr): self.ftable.append([block,expr])
  def addRule(self, rulename):       self.rules.append(rulename)
  def input(self, data):             self.queue.put(data)
  def nexthop(self, data):           return [b for [b,e] in self.ftable if e(data)]
  def run(self):
    while True:
      try:
        odata = self.queue.get()
        idata = odata 
        for r in self.rules:
          odata = r(idata)
        for b in self.nexthop(odata):
          b.input(odata) # forward to first match only
          break
      except Exception as e:
        print('Block ' + self.blkname + ': '  + str(e))

class ExitBlock(Block):
  def addNexthop(self, block, expr): pass
  def addRule(self, rulename): pass
  def run(self):
    context = zmq.Context()
    socket  = context.socket(zmq.PUB)
    socket.bind(ZMQ_OUT_URI)
    while True:
      try:    
        tree = self.queue.get()
        data = ET.tostring(tree)
        socket.send(data)
      except Exception as e: 
        print('Block ' + self.blkname + ': '  + str(e))

class EntryBlock(Block):
  def input(self, data): pass
  def addRule(self, rulename): pass
  def run(self):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.bind(ZMQ_IN_URI)
    socket.setsockopt_string(zmq.SUBSCRIBE, '')
    while True:
      try:    
        data = socket.recv()
        for b in self.nexthop(data):
          tree = ET.fromstring(data)
          b.input(tree) 
          break  # forward only to first matching neighbor 
      except Exception as e: 
        print('Block ' + self.blkname + ': '  + str(e))

class Engine:
  def __init__(self):
    self.blocks = {}
    self.addBlock('entry')
    self.addBlock('exit')
  def addBlock(self, blkname): 
    if blkname not in self.blocks: 
      if blkname == 'entry':
        self.blocks[blkname] = EntryBlock(blkname, self) 
      elif blkname == 'exit':
        self.blocks[blkname] = ExitBlock(blkname, self) 
      else:
        self.blocks[blkname] = Block(blkname, self) 
  def addRuleToBlock(self, blkname, rulename): 
    if blkname in self.blocks: 
      self.blocks[blkname].addRule(rulename)
  def connectBlocks(self, b1, b2, guard):
    if b1 in self.blocks and b2 in self.blocks: 
      self.blocks[b1].addNexthop(self.blocks[b2], guard)
  def start(self):
    for b in self.blocks.values(): b.start()
    while True: 
      try:                       sleep(1)
      except(KeyboardInterrupt): sys.exit(1)
      

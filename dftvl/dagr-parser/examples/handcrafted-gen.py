#!/usr/bin/env python3
from   lark          import Lark, Tree
from   lark.visitors import Transformer, Visitor, Discard
from   lark.lexer    import Lexer, Token
import sys
import zmq
import xml.etree.ElementTree as     ET
from   time                  import sleep
from   queue                 import Queue
from   threading             import Thread
from   pandas                import DataFrame
import dagrlib.closure

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
          if idata is None: break
          odata = r(idata)
          idata = odata
        if odata:
          for b in self.nexthop(odata):
            b.input(odata)
            break  # forward to first match only
      except Exception as e:
        print('Block ' + self.blkname + ': '  + str(e))

class ExitBlock(Block):
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
  def run(self):
    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.bind(ZMQ_IN_URI)
    socket.setsockopt_string(zmq.SUBSCRIBE, '')
    while True:
      try:
        xmldata = socket.recv()
        data = ET.fromstring(xmldata)
        for b in self.nexthop(data):
          b.input(data)
          break  # forward only to first matching neighbor
      except Exception as e:
        print('Block ' + self.blkname + ': '  + str(e))

class Engine:
  def __init__(self):
    self.blocks = { 'entry': EntryBlock('entry', self), 'exit': ExitBlock('exit', self) }
  def add(self, blkname):
    if blkname not in self.blocks: self.blocks[blkname] = Block(blkname, self)
  def connect(self, b1, b2, guard): self.blocks[b1].addNexthop(self.blocks[b2], guard)
  def addRule(self, blkname, rulename): self.blocks[blkname].addRule(rulename)
  def start(self):
    for b in self.blocks.values(): b.start()
    while True:
      try: sleep(1)
      except(KeyboardInterrupt): sys.exit(1)

_xml_namespaces = {}
_xml_namespaces['gma'] = 'urn:gma:1.0'
ET.register_namespace('gma','urn:gma:1.0')

_table_gaps_acl = DataFrame.from_records([
    [1, 1, 1, 'deny'],
    [1, 1, 2, 'allow'],
    [2, 2, 1, 'redact']
  ], 
  columns=['mux', 'sec', 'typ', 'op']
)

def _guard_entry_B1(data):
  return data.find('.//gma:SDHBWPDU',namespaces=_xml_namespaces) != None

def _guard_B1_exit(data):
  return True

def _rule_redact_altitude(data):
  try:
    h = data.find('.//gma:SDHBWHeader',namespaces=_xml_namespaces) 
    m = h.find('./tagm',namespaces=_xml_namespaces) 
    s = h.find('./tags',namespaces=_xml_namespaces) 
    t = h.find('./tagt',namespaces=_xml_namespaces) 
    p = data.find('.//gma:Position',namespaces=_xml_namespaces) 
    x = p.find('./x',namespaces=_xml_namespaces) 
    y = p.find('./y',namespaces=_xml_namespaces) 
    z = p.find('./z',namespaces=_xml_namespaces) 
    if (m.text == str(2)) and (s.text == str(2)) and (t.text == str(1)):
      x.text = str(0.0)
      y.text = str(0.0)
      z.text = str(0.0) 
    return data
  except Exception as e:
    print('_rule_redact_altitude:', str(e))
    return None

def _rule_apply_acl(data):
  try:
    h = data.find('.//gma:SDHBWHeader',namespaces=_xml_namespaces) 
    m = h.find('./tagm',namespaces=_xml_namespaces) 
    s = h.find('./tags',namespaces=_xml_namespaces) 
    t = h.find('./tagt',namespaces=_xml_namespaces) 
    r = [x for _,x in _table_gaps_acl.iterrows() if 
         x['mux'] == int(m.text) and
         x['sec'] == int(s.text) and
         x['typ'] == int(t.text) and
         x['op'] in ['allow','redact']]
    if r == []: return None
    return data
  except Exception as e:
    print('_rule_apply_acl:', str(e))
    return None

if __name__=='__main__':
  e = Engine()
  e.add('B1')
  e.connect('entry','B1',_guard_entry_B1)
  e.connect('B1','exit',_guard_B1_exit)
  e.addRule('B1',_rule_apply_acl)
  e.addRule('B1',_rule_redact_altitude)
  e.start()

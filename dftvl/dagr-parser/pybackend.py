#!/usr/bin/python3
from   lark          import Lark, Tree
from   lark.visitors import Transformer, Visitor, Discard
from   lark.lexer    import Lexer, Token

DAGR_BOILERPLATE='''#!/usr/bin/env python3
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
'''

DAGR_ENGINE='''
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
'''

def rname(r):     return '_rule_%s' % r
def tname(t):     return '_table_%s' % t
def gname(b1,b2): return '_guard_%s_%s' % (b1,b2)
def veval(x):     return x if x.startswith(('r"',"r'",'b"',"b'",'x"',"x'",'c"',"c'")) else eval(x)

def transpile_imports(imp):
  s,n,t  = '','\n','  '
  for i in imp: s += 'import dagrlib.%s' % i + n
  return s

def transpile_namespaces(dct):
  s,n,t  = '','\n','  '
  s += n + '_xml_namespaces = {}' + n
  for k,v in dct.items():
    k1,v1 = veval(k),veval(v)
    s += "_xml_namespaces['%s'] = '%s'" % (k1,v1) + n
    s += "ET.register_namespace('%s','%s')" % (k1,v1) + n
  return s

def transpile_table(q,x):
  (cols,data) = x['thdr'],x['trow']
  s,n,t  = '','\n','  '
  s += n + q + ' = DataFrame.from_records([' + n
  for row in data[:-1]: s += t + t + '%s,' % [veval(y) for y in row] + n
  s += t + t + '%s' % [veval(y) for y in data[-1]] + n
  s += t + '], ' + n
  s += t + 'columns=%s' % cols + n
  s += ')' + n
  return s

def transpile_guard(q,x):
  s,n,t  = '','\n','  '
  s += n + 'def ' + q + '(data):' + n
  # XXX: process actual guard expression here
  #print('pcondition', str(x) if x is not None else x)
  #if x is not None:
  #  for y in x.iter_subtrees_topdown():
  #    print(y.data, [z.data if isinstance(z,Tree) else z.type for z in y.children])
  s += t + 'return True' + n
  return s

def transpile_rule(q,x):
  s,n,t  = '','\n','  '
  s += n + 'def ' + q + '(data):' + n
  # XXX: process actual rule expression here
  #for k,v in x.items():
  #  if k == 'letexp':
  #    for k1,v1 in x[k].items(): print(k1, v1.ltoks(v1))
  #  else:
  #    print(k, v.ltoks(v) if v is not None else v)
  s += t + 'return data' + n
  return s

def python_backend(dagr_ir, output_file):
  once  = [True]
  pline = [(x['srcblk'],x['dstblk'],x['pcondition']) for x in dagr_ir['pline']]
  miss  = [b+'::'+r for b,rs in dagr_ir['rblks'].items() for r in rs if r not in dagr_ir['rules']]
  if len(miss) > 0: raise Exception('Missing definition(s) for block::rule :-\n%s' % '\n'.join(miss))
  if len(dagr_ir['devcs']) != 1: raise Exception('Exactly one device must be specified')
  if veval(dagr_ir['devcs'][0]) != 'CLOSURE': raise Exception('Unhandled device for backend')
  with open(output_file, 'w') as f:
    for _ in once:                           f.write(DAGR_BOILERPLATE)
    for _ in once:                           f.write(transpile_imports(dagr_ir['impts']))
    for _ in once:                           f.write(DAGR_ENGINE)
    for _ in once:                           f.write(transpile_namespaces(dagr_ir['nspcs']))
    for t,x in dagr_ir['tabls'].items():     f.write(transpile_table(tname(t), x))
    for (b1,b2,x) in pline:                  f.write(transpile_guard(gname(b1,b2), x))
    for r,x in dagr_ir['rules'].items():     f.write(transpile_rule(rname(r), x))
    for _ in once:                           f.write("\nif __name__=='__main__':\n")
    for _ in once:                           f.write("  e = Engine()\n")
    for b in dagr_ir['rblks']:               f.write("  e.add('%s')\n" % b)
    for (b1,b2,_) in pline:                  f.write("  e.connect('%s','%s',%s)\n" % (b1,b2,gname(b1,b2)))
    for b,rules in dagr_ir['rblks'].items():
      for r in rules:                        f.write("  e.addRule('%s',%s)\n" % (b,rname(r)))
    for _ in once:                           f.write("  e.start()\n")

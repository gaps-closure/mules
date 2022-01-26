def rname(r):     return '_rule_%s' % r
def tname(t):     return '_table_%s' % t
def gname(b1,b2): return '_guard_%s_%s' % (b1,b2)

def transpile_guard(q,x):
  s,n,t  = '','\n','  '
  s += n + 'def ' + q + '(data):' + n
  # XXX: process actual guard expression here
  s += t + 'return True' + n
  return s

def transpile_rule(q,x):
  s,n,t  = '','\n','  '
  s += n + 'def ' + q + '(data):' + n
  # XXX: process actual rule expression here
  s += t + 'return data' + n
  return s

def transpile_table(q,x):
  s,n,t  = '','\n','  '
  s += n + 'def ' + q + '(data):' + n
  s += t + 'df = DataFrame()' + n
  # XXX: process actual table expression here
  s += t + 'return df' + n
  return s

def python_backend(dagr_ir, output_file):
  pline = list(dagr_ir.pipeline.values())[0]
  once  = [True]
  with open(output_file, 'w') as f: 
    for _ in once:                           f.write("#!/usr/bin/env python3\n")
    for _ in once:                           f.write("from pandas import DataFrame\n")
    for _ in once:                           f.write("from pyengine import Engine\n")
    for (b1,b2,x) in pline:                  f.write(transpile_guard(gname(b1,b2), x))
    for r,x in dagr_ir.rules.items():        f.write(transpile_rule(rname(r), x))
    for t,x in dagr_ir.tables.items():       f.write(transpile_table(tname(t), x))
    for _ in once:                           f.write("\ne = Engine()\n")
    #for r in dagr_ir.rules:                  f.write("e.addRule('%s',%s)\n"  % (r,rname(r))) 
    #for t in dagr_ir.tables:                 f.write("e.addTable('%s',%s)\n" % (t,tname(t))) 
    for b in dagr_ir.ruleblks:               f.write("e.addBlock('%s')\n" % b) 
    for (b1,b2,_) in pline:                  f.write("e.connectBlocks('%s','%s',%s)\n" % (b1,b2,gname(b1,b2)))
    for b,rules in dagr_ir.ruleblks.items():
      for r in rules:                        f.write("e.addRuleToBlock('%s',%s)\n" % (b,rname(r))) 
    for _ in once:                           f.write("e.start()\n")

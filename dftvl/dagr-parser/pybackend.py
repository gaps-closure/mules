def transpile(expr):
  return ''

def python_backend(dagr_ir, output_file):
  with open(output_file, 'w') as f: 
    f.write("#!/usr/bin/env python3\n")
    f.write("from pyengine import Engine\n\n")
    f.write("e = Engine()\n")
    for b in dagr_ir.ruleblks:
      f.write("e.add('%s')\n" % b) 
    # XXX: must transpile and populate blocks here
    for (b1,b2,g) in list(dagr_ir.pipeline.values())[0]:
      f.write("e.connect('%s','%s','%s')\n" % (b1,b2,transpile(g)))
    f.write("e.start()\n")

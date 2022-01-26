#!/usr/bin/python3
from   argparse      import ArgumentParser
from   pprint        import PrettyPrinter
from   lark          import Lark, Tree
from   lark.visitors import Transformer, Interpreter
from   lark.lexer    import Lexer, Token
from   lark.visitors import Discard
from   dagrammar     import DAGR_GRAMMAR
from   pybackend     import python_backend

#---------------------------------------------------------------------------------------------------
def get_args():
  p = ArgumentParser(description='DFDL-Aware Guard Rules Parser')
  p.add_argument('-d', '--dftvl_file',  required=True,  type=str, help='Input DFTVL specification')
  p.add_argument('-s', '--schema_file', required=True,  type=str, help='Input Schema')
  p.add_argument('-o', '--output_file', required=False, type=str, default='gen.py', help='Output filename [gen.py]')
  p.add_argument('-v', '--verbosity',   required=False, type=int, default=0, help='Verbosity [0]')
  p.add_argument('-D', '--dftvl_lang',  required=False, type=str, default='DAGR', help='DFTVL language [DAGR]')
  p.add_argument('-S', '--schema_lang', required=False, type=str, default='DFDL', help='Schema language [DFDL]')
  p.add_argument('-T', '--target_lang', required=False, type=str, default='Python', help='Target for compiler [Python]')
  args = p.parse_args()
  if args.verbosity > 0:
    print('Invoked with following options: ')
    for x in vars(args).items(): print(' --%s %s' % x)
    print('')
  return args

class CleanTokens(Transformer):
  def _hlp(self, items):          return ''.join([x for x in items if isinstance(x, Token)])
  def identifier(self, items):    return Token('ID', self._hlp(items))
  def integer(self, items):       return Token('INT', int(self._hlp(items)))
  def float(self, items):         return Token('FLT', float(self._hlp(items)))
  def bool(self, items):          return Token('BOOL', self._hlp(items)=='true')
  def squotedstring(self, items): return Token('SQSTR', self._hlp(items).strip("'"))
  def dquotedstring(self, items): return Token('DQSTR', self._hlp(items).strip('"'))
  def d(self, items):             return Discard();
  def guard(self, items):         return Discard();
  def arrow(self, items):         return Discard();
  def sdelim(self, items):        return Discard();
  def tdelim(self, items):        return Discard();
  def bstart(self, items):        return Discard();
  def bend(self, items):          return Discard();
  def profile(self, items):       return Discard();
  def pipeline(self, items):      return Discard();
  def proftok(self, items):       return Discard();
  def pipetok(self, items):       return Discard();
  def rblktok(self, items):       return Discard();
  def rdeftok(self, items):       return Discard();
  def tdeftok(self, items):       return Discard();
  def cif(self, items):           return Discard();
  def cthen(self, items):         return Discard();
  def celse(self, items):         return Discard();

def gs1(t,d): return list(t.find_data(d))[0].children[0].value
def gs2(t,d): return list(t.find_data(d))[0].children[0].children[0].value
def gv1(t,d): return list([x.children[0].value for x in t.find_data(d)])

def gc1(t,d): 
  l = list(t.find_data(d))
  return l[0].children if len(l) == 1 else []

def banprt(m,x):
  print('-----------------------------------------------')
  print(m)
  print('-----------------------------------------------')
  PrettyPrinter().pprint(x)

class Frontend_DAGR():
  def __init__(self,tree,verbosity=0):
    for n in tree.iter_subtrees_topdown(): 
      if isinstance(n, Tree): n.children = list(filter(lambda x: not(isinstance(x,Discard)), n.children))

    self.devices  = [e.children[1].children[0].value for p in tree.find_data('profblk')
                     for e in p.find_data('profelt') if e.children[0].data == 'device']
    self.globals  = [e.children[1].children[0].value for p in tree.find_data('profblk')
                     for e in p.find_data('profelt') if e.children[0].data == 'global']
    self.nspaces  = { e.children[1].children[0].value:e.children[2].children[0].value
                      for p in tree.find_data('profblk')
                      for e in p.find_data('profelt') if e.children[0].data == 'namespace'}
    self.pipeline = { gs1(p,'pipename') : [(gs2(x,'srcblk'), gs2(x,'dstblk'), gc1(x,'condition')) 
                                            for x in p.find_data('connector')]
                      for p in tree.find_data('pipeblk') }
    self.ruleblks = { gs1(x,'rblkname') : gv1(x,'rulename') for x in tree.find_data('ruleblk') }
    self.tables   = { gs1(t, 'tblname') : ([gv1(h, 'colname') for h in t.find_data('thdr')][0], 
                                           [gv1(r, 'dagrval') for r in t.find_data('trow')])
                      for t in tree.find_data('tbldef') }
    self.rules    = { gs1(p,'rulename') : gc1(p,'rexpr') for p in tree.find_data('ruledef') }

    for bname,rules in self.ruleblks.items():
      for rname in rules:
        if rname not in self.rules:
          raise Exception('Missing definition for rule %s needed by block %s' % (rname,bname))

    if verbosity > 0: 
      banprt('Abstract Syntax Tree from Parser:', '')
      print(tree.pretty() if verbosity <= 2 else tree)

    if verbosity > 1:
      banprt('Intermediate Representation - Devices:',     self.devices)
      banprt('Intermediate Representation - Globals:',     self.globals)
      banprt('Intermediate Representation - Namespaces:',  self.nspaces)
      banprt('Intermediate Representation - Pipeline:',    self.pipeline)
      banprt('Intermediate Representation - Rule blocks:', self.ruleblks)
      banprt('Intermediate Representation - Tables:',      self.tables)
      banprt('Intermediate Representation - Rules:',       self.rules)

#---------------------------------------------------------------------------------------------------
if __name__ == '__main__':
  args   = get_args()

  if args.schema_lang == 'DFDL':
    pass
  elif args.schema_lang == 'XSD':
    raise Exception('Not yet implemented:' + args.schema_lang)
  else:
    raise Exception('Unsupported schema language:' + args.schema_lang)

  if args.dftvl_lang == 'DAGR':
    parser = Lark(DAGR_GRAMMAR, start='spec', parser='lalr', lexer='contextual', transformer=CleanTokens())
    with open(args.dftvl_file, 'r') as inf: ast = parser.parse(inf.read())
    dagr_ir = Frontend_DAGR(ast, verbosity=args.verbosity)
  else:
    raise Exception('Unsupportd DFTVL language:' + args.dftvl_lang)

  if args.target_lang == 'Python':
    python_backend(dagr_ir, args.output_file)
  else:
    raise Exception('Unsupported compilation target:' + args.target_lang)


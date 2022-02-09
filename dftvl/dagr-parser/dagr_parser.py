#!/usr/bin/python3
from   argparse      import ArgumentParser
from   pprint        import PrettyPrinter
from   lark          import Lark, Tree
from   lark.visitors import Transformer, Interpreter
from   lark.lexer    import Lexer, Token
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
  def regesxtring(self, items):   return Token('REGEX', self._hlp(items).lstrip("r").strip("'"))
  def xpathstring(self, items):   return Token('XPATH', self._hlp(items).lstrip("x").strip("'"))
  def cselstring(self, items):    return Token('CSEL', self._hlp(items).lstrip("c").strip("'"))
  def unistring(self, items):     return Token('CSEL', self._hlp(items).lstrip("u").strip("'"))
  def bytestring(self, items):    return Token('CSEL', self._hlp(items).lstrip("b").strip("'"))
  def squotedstring(self, items): return Token('SQSTR', self._hlp(items).strip("'"))
  def dquotedstring(self, items): return Token('DQSTR', self._hlp(items).strip('"'))


class Frontend_DAGR():
  def __init__(self,tree,verbosity=0):
    def fct(t,d):     return t.children[0].data == d
    def fmfcv(t,d):   return list(t.find_data(d))[0].children[0].value
    def fmfcfcv(t,d): return list(t.find_data(d))[0].children[0].children[0].value
    def amfcv(t,d):   return list([x.children[0].value for x in t.find_data(d)])
    def fmac(t,d):    return list([x.children for x in t.find_data(d)])[:1]
    def edges(t):     return [(fmfcfcv(x,'srcblk'),fmfcfcv(x,'dstblk'),fmac(x,'condition')) for x in t.find_data('connector')]
    def hdr(t):       return [amfcv(x, 'colname') for x in t.find_data('thdr')][0] 
    def rows(t):      return [amfcv(x, 'dagrval') for x in t.find_data('trow')]

    self.devices  = [ fmfcv(x,'devname')                        for x in tree.find_data('profelt') if fct(x,'device') ]
    self.globals  = [ fmfcv(x,'gvarname')                       for x in tree.find_data('profelt') if fct(x,'global') ]
    self.nspaces  = { fmfcv(x,'nsalias')  : fmfcv(x,'nspath')   for x in tree.find_data('profelt') if fct(x,'namespace') }
    self.pipeline = { fmfcv(x,'pipename') : edges(x)            for x in tree.find_data('pipeblk') }
    self.ruleblks = { fmfcv(x,'rblkname') : amfcv(x,'rulename') for x in tree.find_data('ruleblk') }
    self.tables   = { fmfcv(x,'tblname')  : (hdr(x), rows(x))   for x in tree.find_data('tbldef') }
    self.rules    = { fmfcv(x,'rulename') : fmac(x,'rexpr')     for x in tree.find_data('ruledef') }

    miss = [b+'::'+r for b,rs in self.ruleblks.items() for r in rs if r not in self.rules]
    if len(miss) > 0: raise Exception('Missing definition(s) for block::rule :-\n%s' % '\n'.join(miss))

    self.log(tree, verbosity)

  def log(self, tree, verbosity):
    def banprt(m,x):
      print('-' * 50 + '\n' + m + '\n' + '-' * 50)
      if x: PrettyPrinter().pprint(x)

    if verbosity > 0: 
      banprt('Abstract Syntax Tree from Parser:', None)
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


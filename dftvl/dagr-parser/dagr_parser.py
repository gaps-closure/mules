#!/usr/bin/python3
from   argparse      import ArgumentParser
from   pprint        import pformat
from   lark          import Lark, Tree
from   lark.visitors import Transformer, Visitor, Discard
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
    print('Invoked with following command line options: ')
    for x in vars(args).items(): print(' --%s %s' % x)
    print('')
  return args

#---------------------------------------------------------------------------------------------------
def _hlp(i):  return ''.join([x for x in i if isinstance(x, Token)])
def _ehlp(i): return _hlp(i).replace('\\"','"').replace("\\'","'")
def _bhlp(i): return _hlp(i).replace('`','')

class CleanTokens(Transformer):
  def identifier(self,i): return Token('SID',   _bhlp(i))
  def complexid(self,i):  return Token('CID',   _bhlp(i))
  def nil(self,i):        return Token('NIL',   None)
  def bool(self,i):       return Token('BOOL',  _hlp(i)=='True')
  def integer(self,i):    return Token('INT',   int(_hlp(i)))
  def float(self,i):      return Token('FLT',   float(_hlp(i)))
  def string(self,i):     return Token('STRNG', _ehlp(i)[1:-1])
  def ustring(self,i):    return Token('UNSTR', _ehlp(i)[2:-1])
  def bstring(self,i):    return Token('BYSTR', _ehlp(i))
  def rstring(self,i):    return Token('REGEX', _ehlp(i))
  def xstring(self,i):    return Token('XPATH', _ehlp(i))
  def cstring(self,i):    return Token('CSSEL', _ehlp(i))

def m(t,d):        return t.find_data(d)
def f(y):          return [x.children[0] for x in y]
def v(y):          return [x.value       for x in y]
def idfs(t,d):     return v(f(m(t,d)))
def idf(t,d):      return idfs(t,d)[0]
def rows(t,d,y):   return [v(f(m(x,y))) for x in m(t,d)]
def row(t,d,y):    return rows(t,d,y)[0]
def rool(t,a,b,c): return (f(m(t,a)), f(m(t,b)), f(m(t,c))[:1])
def edge(t,a,b,c): return (v(f(f(m(t,a))))[0], v(f(f(m(t,b))))[0], f(m(t,c))[:1])

class Frontend_DAGR(Visitor):
  def devprof(self,x):   self.d['devcs'].append(idf(x,'devname'))
  def glprof(self,x):    self.d['glbls'].append(idf(x,'gvarname'))
  def improf(self,x):    self.d['impts'].append(idf(x,'libname'))
  def connector(self,x): self.d['pline'].append(edge(x,'srcblk','dstblk','pcondition'))
  def nsprof(self,x):    self.d['nspcs'].update({ idf(x,'nsalias')  : idf(x,'nspath') })
  def ruleblk(self,x):   self.d['rblks'].update({ idf(x,'rblkname') : idfs(x,'rulename') })
  def tbldef(self,x):    self.d['tabls'].update({ idf(x,'tblname')  : (row(x,'thdr','colname'), rows(x,'trow','dagrval')) })
  def ruledef(self,x):   self.d['rools'].update({ idf(x,'rulename') : (rool(x,'condition','action','altaction')) })
  def __init__(self,tree,verbosity):
    self.d =  dict(devcs=[],glbls=[],impts=[],nspcs={},pline=[],rblks={},tabls={},rools={})
    super().__init__()
    self.visit(tree)
    miss = [b+'::'+r for b,rs in self.d['rblks'].items() for r in rs if r not in self.d['rools']]
    if len(miss) > 0: raise Exception('Missing definition(s) for block::rule :-\n%s' % '\n'.join(miss))
    def banprt(m,x):  print('\n'.join(['-'*50, m, '-'*50, x, '']))
    if verbosity > 0: 
      banprt('Abstract Syntax Tree from Parser:', (tree.pretty() if verbosity <= 2 else tree))
    if verbosity > 1: 
      for k,v in self.d.items(): banprt('Intermediate Representation - %s:' % k, pformat(v))

#---------------------------------------------------------------------------------------------------
if __name__ == '__main__':
  args   = get_args()

  if args.schema_lang == 'DFDL':
    # XXX: generate DFDL <xpath,type,offset,len> map here
    pass
  elif args.schema_lang == 'XSD':
    raise Exception('Not yet implemented:' + args.schema_lang)
  else:
    raise Exception('Unsupported schema language:' + args.schema_lang)

  if args.dftvl_lang == 'DAGR':
    parser = Lark(DAGR_GRAMMAR, start='spec', parser='lalr', lexer='contextual', transformer=CleanTokens())
    with open(args.dftvl_file, 'r') as inf: ast = parser.parse(inf.read())
    dagr_ir = Frontend_DAGR(ast,args.verbosity).d
  else:
    raise Exception('Unsupportd DFTVL language:' + args.dftvl_lang)

  if args.target_lang == 'Python':
    python_backend(dagr_ir, args.output_file)
  else:
    raise Exception('Unsupported compilation target:' + args.target_lang)

#---------------------------------------------------------------------------------------------------

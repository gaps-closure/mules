#!/usr/bin/python3
from   argparse      import ArgumentParser
from   pprint        import pformat
from   lark          import Lark, Tree
from   lark.visitors import Transformer, Visitor, Interpreter
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
  if args.verbosity > 1:
    print('Invoked with following command line options: ')
    for x in vars(args).items(): print(' --%s %s' % x)
    print('')
  return args

#---------------------------------------------------------------------------------------------------
# Utility functions
def banprt(m,x):          print('\n'.join(['-'*50, m, '-'*50, x]))
def hlp(i):               return ''.join([x for x in i if isinstance(x, Token)])
def ehlp(i):              return hlp(i).replace('\\"','"').replace("\\'","'")
def bhlp(i):              return hlp(i).replace('`','')
def em(x):                return [ExprMunge().transform(y) for y in x]
def m(x,d):               return [y for y in x.find_data(d)]
def f(x):                 return [y.children[0] for y in x]
def v(x):                 return [y.value for y in x]
def idfs(x,d):            return v(f(m(x,d)))
def idf(x,d):             return idfs(x,d)[0]
def rows(x,d,c):          return [v(f(m(y,c))) for y in m(x,d)]
def row(x,d,c):           return rows(x,d,c)[0]
def rool(x,d):            return em(m(x,d))[0]
def edge(x,s,d,g):        return (v(f(f(m(x,s))))[0], v(f(f(m(x,d))))[0], em(f(m(x,g))))

# Called by parser for munging identifiers and primitive types
class CleanTokens(Transformer):
  def identifier(self,x): return Token('SID',   bhlp(x))
  def complexid(self,x):  return Token('CID',   bhlp(x))
  def nil(self,x):        return Token('NIL',   None)
  def bool(self,x):       return Token('BOOL',  hlp(x)=='True')
  def integer(self,x):    return Token('INT',   int(hlp(x)))
  def float(self,x):      return Token('FLT',   float(hlp(x)))
  def string(self,x):     return Token('STRNG', ehlp(x)[1:-1])
  def ustring(self,x):    return Token('UNSTR', ehlp(x)[2:-1])
  def bstring(self,x):    return Token('BYSTR', ehlp(x))
  def rstring(self,x):    return Token('REGEX', ehlp(x))
  def xstring(self,x):    return Token('XPATH', ehlp(x))
  def cstring(self,x):    return Token('CSSEL', ehlp(x))

# Called by Frontend_DAGR to transform expressions
class ExprMunge(Transformer):
  def rexpr(self,x):      return Tree('rexpr', [y for y in x if isinstance(y,Tree)])
  def letexp(self,x):     return Tree('letexp', [x[1].children[0],x[2]])
  def ablock(self,x):     return Tree('ablock', [y for y in x if isinstance(y,Tree) and y.data=='function'])
  def action(self,x):     return Tree('action', x[0].children)
  def altaction(self,x):  return Tree('altaction', x[0].children)

# Collects the DAGR specification internal representation in a dictionary
class Frontend_DAGR(Visitor):
  def connector(self,x):  self.d['pline'].append(edge(x,'srcblk','dstblk','pcondition'))
  def devprof(self,x):    self.d['devcs'].append(idf(x,'devname'))
  def glprof(self,x):     self.d['glbls'].append(idf(x,'gvarname'))
  def improf(self,x):     self.d['impts'].append(idf(x,'libname'))
  def nsprof(self,x):     self.d['nspcs'].update({ idf(x,'nsalias')  : idf(x,'nspath') })
  def ruleblk(self,x):    self.d['rblks'].update({ idf(x,'rblkname') : idfs(x,'rulename') })
  def ruledef(self,x):    self.d['rools'].update({ idf(x,'rulename') : rool(x,'rexpr') })
  def tbldef(self,x):     self.d['tabls'].update({ idf(x,'tblname')  : (row(x,'thdr','colname'), rows(x,'trow','dagrval')) })
  def __init__(self,tree,verbosity):
    self.d =  dict(devcs=[],glbls=[],impts=[],nspcs={},pline=[],rblks={},tabls={},rools={})
    super().__init__()
    self.visit(tree)

#---------------------------------------------------------------------------------------------------
if __name__ == '__main__':
  args = get_args()

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
    if args.verbosity > 1: banprt('Abstract Syntax Tree from Parser:', (ast.pretty() if args.verbosity <= 2 else ast))
    if args.verbosity > 0: banprt('Intermediate Representation:', pformat(dagr_ir))
  else:
    raise Exception('Unsupportd DFTVL language:' + args.dftvl_lang)

  if args.target_lang == 'Python':
    python_backend(dagr_ir, args.output_file)
  else:
    raise Exception('Unsupported compilation target:' + args.target_lang)

#---------------------------------------------------------------------------------------------------

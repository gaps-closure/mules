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
  if args.verbosity > 1:
    print('Invoked with following command line options: ')
    for x in vars(args).items(): print(' --%s %s' % x)
    print('')
  return args

#---------------------------------------------------------------------------------------------------
# Utility functions
def detok(x):             return [y for y in x if isinstance(y,Tree)]
def hlp(i):               return ''.join([x for x in i if isinstance(x, Token)])
def ehlp(i):              return hlp(i).replace('\\"','"').replace("\\'","'")
def bhlp(i):              return hlp(i).replace('`','')
def w(x):                 return TokenLister(x.data,x.children) if isinstance(x,Tree) else x
def ew(x):                return w(x[0]) if len(x) > 0 else None
def m(x,d):               return [y for y in x.find_data(d)]
def f(x):                 return [y.children[0] for y in x]
def v(x):                 return [y.value for y in x]
def idfs(x,d):            return v(f(m(x,d)))
def idf(x,d):             return idfs(x,d)[0]
def rows(x,d,c):          return [v(f(m(y,c))) for y in m(x,d)]
def tabl(x,h,c,r,v):      return {h:rows(x,h,c)[0], r:rows(x,r,v)}
def letdict(x):           return {y.children[0].children[0].value:w(y.children[1]) for y in x}
def rool(x,l,i,t,e):      return {l:letdict(m(x,l)), i:w(f(m(x,i))[0]), t:w(f(m(x,t))[0]), e:ew(f(m(x,e)))}
def edge(x,s,d,g):        return {s:v(f(f(m(x,s))))[0], d:v(f(f(m(x,d))))[0], g:ew(f(m(x,g)))}

class TokenLister(Tree):
  def __repr__(self):     return '""" ' + ' '.join(self.ltoks(self)) + ' """'
  def ltoks(self,x):      return [x.value] if isinstance(x,Token) else [i for y in x.children for i in self.ltoks(y)]

# Transformer munges identifiers and primitives, and removes redundant tokens
class CleanTokens(Transformer):
  def identifier(self,x): return Token('SID', bhlp(x))
  def complexid(self,x):  return Token('CID', bhlp(x))
  def nil(self,x):        return Token('NIL', hlp(x))
  def integer(self,x):    return Token('INT', hlp(x))
  def float(self,x):      return Token('FLT', hlp(x))
  def bool(self,x):       return Token('BOOL', hlp(x))
  def string(self,x):     return Token('STRNG', ehlp(x))
  def ustring(self,x):    return Token('UNSTR', ehlp(x))
  def bstring(self,x):    return Token('BYSTR', ehlp(x))
  def rstring(self,x):    return Token('REGEX', ehlp(x))
  def xstring(self,x):    return Token('XPATH', ehlp(x))
  def cstring(self,x):    return Token('CSSEL', ehlp(x))
  def profblk(self,x):    return Tree('profblk', detok(x))
  def pipeblk(self,x):    return Tree('profblk', detok(x))
  def connector(self,x):  return Tree('connector', detok(x))
  def ruleblk(self,x):    return Tree('ruleblk', detok(x))
  def tbldef(self,x):     return Tree('tbldef', detok(x))
  def thdr(self,x):       return Tree('thdr', detok(x))
  def trow(self,x):       return Tree('trow', detok(x))
  def ruledef(self,x):    return Tree('ruledef', detok(x))
  def letexp(self,x):     return Tree('letexp', detok(x))
  def rexpr(self,x):      return Tree('rexpr', detok(x))
  def ablock(self,x):     return Tree('ablock', detok(x))

# Visitor collects DAGR specification in a dict-based internal representation
class Frontend_DAGR(Visitor):
  def connector(self,x):  self.d['pline'].append(edge(x,'srcblk','dstblk','pcondition'))
  def devprof(self,x):    self.d['devcs'].append(idf(x,'devname'))
  def glprof(self,x):     self.d['glbls'].append(idf(x,'gvarname'))
  def improf(self,x):     self.d['impts'].append(idf(x,'libname'))
  def nsprof(self,x):     self.d['nspcs'].update({idf(x,'nsalias'): idf(x,'nspath')})
  def ruleblk(self,x):    self.d['rblks'].update({idf(x,'rblkname'): idfs(x,'rulename')})
  def tbldef(self,x):     self.d['tabls'].update({idf(x,'tblname'): tabl(x,'thdr','colname', 'trow','dagrval')})
  def ruledef(self,x):    self.d['rules'].update({idf(x,'rulename'): rool(x,'letexp','condition','action','altaction')})
  def __init__(self,tree,verbosity):
    self.d =  dict(devcs=[],glbls=[],impts=[],nspcs={},pline=[],rblks={},tabls={},rules={})
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
    if args.verbosity > 1: print('Abstract Syntax Tree:\n', (ast.pretty() if args.verbosity <= 2 else ast))
    if args.verbosity > 0: print('Intermediate Representation:\n', pformat(dagr_ir, sort_dicts=False))
  else:
    raise Exception('Unsupportd DFTVL language:' + args.dftvl_lang)

  if args.target_lang == 'Python':
    python_backend(dagr_ir, args.output_file)
  else:
    raise Exception('Unsupported compilation target:' + args.target_lang)

#---------------------------------------------------------------------------------------------------

#!/usr/bin/python3
from   argparse      import ArgumentParser
from   pprint        import PrettyPrinter
from   lark          import Lark, Tree
from   lark.visitors import Transformer, Interpreter
from   lark.lexer    import Lexer, Token
from   lark.visitors import Discard
from   dagrammar     import DAGR_GRAMMAR

#---------------------------------------------------------------------------------------------------
def get_args(showargs=False):
  p = ArgumentParser(description='DFDL-Aware Gaur Rules Parser')
  p.add_argument('-f', '--file', required=True, type=str, help='Input file')
  args = p.parse_args()
  if showargs:
    print('Invoked with following options: ')
    for x in vars(args).items(): print(' --%s %s' % x)
    print('')
  return args

class CleanTokens(Transformer):
  def _hlp(self, items):         return ''.join([x for x in items if isinstance(x, Token)])
  def identifier(self, items):   return Token('ID', self._hlp(items))
  def integer(self, items):      return Token('INT', self._hlp(items))
  def d(self, items):            return Discard();
  def guard(self, items):        return Discard();
  def arrow(self, items):        return Discard();
  def sdelim(self, items):       return Discard();
  def tdelim(self, items):       return Discard();
  def bstart(self, items):       return Discard();
  def bend(self, items):         return Discard();
  def profile(self, items):      return Discard();
  def pipeline(self, items):     return Discard();
  def proftok(self, items):      return Discard();
  def pipetok(self, items):      return Discard();
  def rblktok(self, items):      return Discard();
  def rdeftok(self, items):      return Discard();
  def tdeftok(self, items):      return Discard();
  def cif(self, items):          return Discard();
  def cthen(self, items):        return Discard();
  def celse(self, items):        return Discard();

def gs1(t,d): return list(t.find_data(d))[0].children[0].value
def gs2(t,d): return list(t.find_data(d))[0].children[0].children[0].value
def gv1(t,d): return list([x.children[0].value for x in t.find_data(d)])

def gc1(t,d): 
  l = list(t.find_data(d))
  return l[0].children if len(l) == 1 else []
  
class DAGRProcessor():
  def __init__(self,tree,discard=True,showast=True,pretty=True): 
    if discard:
      for n in tree.iter_subtrees_topdown(): 
        if isinstance(n, Tree): n.children = list(filter(lambda x: not(isinstance(x,Discard)), n.children))
    if showast: 
      print(tree.pretty()) if pretty else print(tree)

    self.profile  = tree.find_data('profblk')
    self.tables   = tree.find_data('tbldef')

    self.ruleblks = { gs1(x,'rblkname') : gv1(x,'rulename') for x in tree.find_data('ruleblk') }
    self.pipeline = { gs1(p,'pipename') : [ ( gs2(x,'srcblk'), gs2(x,'dstblk'), gc1(x,'condition') ) 
                                            for x in p.find_data('connector')]
                      for p in tree.find_data('pipeblk') }
    self.rules    = { gs1(p,'rulename') : gc1(p,'rexpr') for p in tree.find_data('ruledef') }

#---------------------------------------------------------------------------------------------------
if __name__ == '__main__':
  args   = get_args(showargs=True)
  parser = Lark(DAGR_GRAMMAR, start='spec', parser='lalr', lexer='contextual', transformer=CleanTokens())
  with open(args.file, 'r') as inf: ast = parser.parse(inf.read())
  proc   = DAGRProcessor(ast, discard=True, showast=True, pretty=True)

  PrettyPrinter().pprint(proc.pipeline)
  PrettyPrinter().pprint(proc.ruleblks)
  PrettyPrinter().pprint(proc.rules)
   

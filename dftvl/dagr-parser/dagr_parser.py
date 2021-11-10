#!/usr/bin/python3
from   argparse      import ArgumentParser
from   lark          import Lark, Tree
from   lark.visitors import Transformer
from   lark.lexer    import Lexer, Token
from   lark.visitors import Discard
from   dagrammar     import DAGR_GRAMMAR

#---------------------------------------------------------------------------------------------------
def get_args():
  p = ArgumentParser(description='DFDL-Aware Gaur Rules Parser')
  p.add_argument('-f', '--file', required=True, type=str, help='Input file')
  return p.parse_args()

def dagr_parser(): 
  return Lark(DAGR_GRAMMAR, start='spec', parser='lalr', lexer='contextual')

class DAGRTransformer(Transformer):
  def _hlp(self, items):         return ''.join([x for x in items if isinstance(x, Token)])

  def guard(self, items):        return Discard();
  def arrow(self, items):        return Discard();
  def sdelim(self, items):       return Discard();
  def tdelim(self, items):       return Discard();
  def bstart(self, items):       return Discard();
  def bend(self, items):         return Discard();
  def profile(self, items):      return Discard();
  def pipeline(self, items):     return Discard();
  def ruleblk(self, items):      return Discard();
  def proftok(self, items):      return Discard();
  def pipetok(self, items):      return Discard();
  def rblktok(self, items):      return Discard();
  def rdeftok(self, items):      return Discard();
  def cif(self, items):          return Discard();
  def cthen(self, items):        return Discard();
  def celse(self, items):        return Discard();
  def d(self, items):            return Discard();

  def identifier(self, items):   return Token('ID', self._hlp(items))
  def integer(self, items):      return Token('INT', self._hlp(items))

#---------------------------------------------------------------------------------------------------
if __name__ == '__main__':
  args = get_args()
  print('Invocation options selected:')
  for x in vars(args).items(): print('  %s: %s' % x)
  with open(args.file, 'r') as inf: instr = inf.read()

  ast = dagr_parser().parser.parse(instr)
  tst = DAGRTransformer().transform(ast)
  for n in tst.iter_subtrees_topdown(): 
    if isinstance(n, Tree): n.children = list(filter(lambda x: not(isinstance(x,Discard)), n.children))
  print(tst.pretty())



#!/usr/bin/python3
from   argparse      import ArgumentParser
from   lark          import Lark, Tree
from   lark.visitors import Transformer
from   lark.lexer    import Lexer, Token
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
  def _ign(self):                return Token('IGNORE', '')

  def guard(self, items):        return self._ign()
  def arrow(self, items):        return self._ign()
  def sdelim(self, items):       return self._ign()
  def tdelim(self, items):       return self._ign()
  def bstart(self, items):       return self._ign()
  def bend(self, items):         return self._ign()
  def profile(self, items):      return self._ign()
  def pipeline(self, items):     return self._ign()
  def ruleblk(self, items):      return self._ign()
  def proftok(self, items):      return self._ign()
  def pipetok(self, items):      return self._ign()
  def rblktok(self, items):      return self._ign()
  def rdeftok(self, items):      return self._ign()
  def d(self, items):            return self._ign()

  def identifier(self, items):   return Token('ID', self._hlp(items))

  #def pfx(self, items):      return items[0].value.extent.start.line
  #def foo(self, items):      return [i for s in items for i in s]
  #def cledef(self, items):   return [['cledef'] + items]


#---------------------------------------------------------------------------------------------------
if __name__ == '__main__':
  args   = get_args()
  print('Options selected:')
  for x in vars(args).items(): print('  %s: %s' % x)

  with open(args.file, 'r') as inf:
    instr = inf.read()

  tree = dagr_parser().parser.parse(instr)
  #print(tree)
  #print(tree.pretty())
  #for n in tree.iter_subtrees_topdown(): print(n.data)

  ttree  = DAGRTransformer().transform(tree)
  #print(ttree)
  print(ttree.pretty())


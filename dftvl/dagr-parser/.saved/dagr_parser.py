#!/usr/bin/python3
from   argparse      import ArgumentParser
from   lark          import Lark, Tree
from   lark.visitors import Transformer
from   lark.lexer    import Lexer, Token
from   dagrammar     import DAGR_GRAMMAR

#---------------------------------------------------------------------------------------------------
# Grammar and parser for DAGR
def dagr_parser():
  return Lark(DAGR_GRAMMAR, start='specification', parser='lalr', lexer='contextual')

# Parse command line argumets
def get_args():
  p = ArgumentParser(description='DFDL-Aware Gaur Rules Parser')
  p.add_argument('-f', '--file', required=True, type=str, help='Input file')
  return p.parse_args()

#---------------------------------------------------------------------------------------------------
if __name__ == '__main__':
  args   = get_args()
  print('Options selected:')
  for x in vars(args).items(): print('  %s: %s' % x)

  with open(args.file, 'r') as inf:
    instr = inf.read()

  tree = dagr_parser().parser.parse(instr)

  print(tree.pretty())
  #for n in tree.iter_subtrees_topdown(): print(n.data)


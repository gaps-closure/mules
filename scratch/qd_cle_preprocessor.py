#!/usr/bin/python3
# A quick and dirty cle-preprocessor implementation for GAPS-CLOSURE
#
from   clang.cindex  import Index, TokenKind
from   lark.lexer    import Lexer, Token
from   argparse      import ArgumentParser
from   lark          import Lark, Tree
from   lark.visitors import Transformer
import json
import sys
import os

# Invoke libclang tokenizer
def cindex_tokenizer(f,a):
  return Index.create().parse(f,args=a).cursor.get_tokens()

# Transform tokens for CLE parsing
class TypeLexer(Lexer):
  def __init__(self, lexer_conf): pass
  def lex(self, data):
    for x in data:
      if x.kind == TokenKind.PUNCTUATION:
        yield Token('HASH' if x.spelling=='#' else 'PUNCT', x)
      elif x.kind == TokenKind.IDENTIFIER:
        if   x.spelling == 'pragma': yield Token('PRAGMA', x)
        elif x.spelling == 'cle':    yield Token('CLE', x)
        elif x.spelling == 'def':    yield Token('DEF', x)
        elif x.spelling == 'begin':  yield Token('BEGIN', x)
        elif x.spelling == 'end':    yield Token('END', x)
        else:                        yield Token('IDENT', x)
      elif x.kind == TokenKind.LITERAL:
        yield Token('LITERAL', x)
      elif x.kind == TokenKind.COMMENT:
        yield Token('COMMENT', x)
      elif x.kind == TokenKind.KEYWORD:
        yield Token('KWD', x)
      else:
        raise TypeError(x)

# Grammar and parser for CLE
def cle_parser():
  return Lark(r"""
    acode:       acode_item+
    ?acode_item: cdirective
                 | other
    ?cdirective: cledef
                 | clebegin
                 | cleend
                 | cleappnl
    cleappnl:    pfx label
    clebegin:    pfx begin label
    cleend:      pfx end label
    cledef:      pfx deff label clejson
    pfx:         HASH PRAGMA CLE
    begin:       BEGIN
    end:         END
    deff:        DEF
    label:       IDENT
    clejson:     LITERAL
                 | PUNCT (PUNCT | LITERAL)+
    other:       COMMENT
                 | nonhash+
                 | HASH (PRAGMA | IDENT | KWD)
                 | HASH HASH (IDENT | KWD)
    ?nonhash:    PUNCT
                 | KWD 
                 | LITERAL 
                 | IDENT
                 | PRAGMA
                 | CLE
                 | DEF
                 | BEGIN
                 | END
    %declare PUNCT COMMENT KWD LITERAL IDENT HASH PRAGMA CLE DEF BEGIN END
  """, start='acode', parser='lalr', lexer=TypeLexer)

# Tranform parsed tree to extract relevant CLE information
class CLETransformer(Transformer):
  def _hlp(self, items):
    return ' '.join([x.value.spelling for x in items if isinstance(x, Token)])
  def acode(self, items):    return [i for s in items for i in s]
  def other(self, items):    return []
  def begin(self, items):    return []
  def end(self, items):      return []
  def deff(self, items):     return []
  def pfx(self, items):      return items[0].value.extent.start.line
  def label(self, items):    return self._hlp(items)
  def clejson(self, items):  return json.loads(self._hlp(items))
  def cledef(self, items):   return [['cledef'] + items]
  def clebegin(self, items): return [['clebegin'] + items]
  def cleend(self, items):   return [['cleend'] + items]
  def cleappnl(self, items): return [['cleappnl'] + items]

# Based on transformed tree create modified source and mappings file
def source_transform(infile,ttree):
  # Collect cledefs and dump
  defs = [{"cle-label": x[3], "cle-json": x[4]} for x in ttree if x[0] == 'cledef']
  with open(infile + ".clemap.json", 'w') as mapf:
    json.dump(defs,mapf,indent=2)

  curline = 0
  with open(infile) as inf:
    fn,fe = os.path.splitext(infile)
    with open(fn + '.mod' + fe,'w') as ouf:
      for x in sorted(ttree, key=lambda x: x[1]):
        if x[0] == 'clebegin':
          while curline < x[1]: 
            ouf.write(inf.readline())
            curline += 1
          ouf.write('#pragma clang attribute push (__attribute__((annotate("')
          ouf.write(x[3])
          ouf.write('"))), apply_to = any(function,type_alias,record,enum,variable,field))')
          ouf.write('\n')
        elif x[0] == 'cleend':
          while curline < x[1] - 1: 
            ouf.write(inf.readline())
            curline += 1
          ouf.write('#pragma clang attribute pop\n')
          ouf.write(inf.readline())
          curline +=1
        elif x[0] == 'cleappnl':
          while curline < x[1]: 
            ouf.write(inf.readline())
            curline += 1
          print('cleappnl not implemented:', x)
        else:
          pass
      # Copy remaining lines if any
      for line in inf: 
        ouf.write(line)
        curline += 1
    
# Parse command line argumets
def get_args():
  p = ArgumentParser(description='CLOSURE Language Extensions Preprocessor')
  p.add_argument('-f', '--file', required=True, type=str, help='Input file')
  p.add_argument('-c', '--clang_args', required=False, type=str, 
                 default='-x,c++,-stdlib=libc++', help='Arguments for clang')
  p.add_argument('-a', '--annotation_style', required=False, type=str, 
                 default='both', help='Annotation style (naive, type, or both)')
  p.add_argument('-t', '--tool_chain', required=False, type=str, 
                 default='clang', help='Toolchain (clang)')
  return p.parse_args()

# Create and invoke tokenizer, parser, tree transformer, and source transformer
def main():
  args   = get_args()
  print('Options selected:')
  for x in vars(args).items(): print('  %s: %s' % x)

  toks   = cindex_tokenizer(args.file, args.clang_args.split(','))
  tree   = cle_parser().parser.parse(toks)
  print(tree.pretty())
  print('Transformed Tree:')

  ttree  = CLETransformer().transform(tree)
  for x in ttree: print(x)

  source_transform(args.file, ttree)
  print('Writing transformed file and cle mappings file')

if __name__ == '__main__':
  main()

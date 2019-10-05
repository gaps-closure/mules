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

def cindex_tokenizer(f,a):
  return Index.create().parse(f,args=a).cursor.get_tokens()

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

def main():
  args = get_args()
  # for x in vars(args).items(): print('%s: %s' % x)
  toks = cindex_tokenizer(args.file, args.clang_args.split(','))
  parser = cle_parser()

  tree = parser.parse(toks)
  # print(tree.pretty())
  tt = CLETransformer().transform(tree)
  for x in tt: print(x)

if __name__ == '__main__':
  main()

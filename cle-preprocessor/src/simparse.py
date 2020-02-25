import sys
import clang.cindex

def srcrangestr(x):
    return '%s:%d:%d - %s:%d:%d' % (x.start.file, x.start.line, x.start.column, x.end.file, x.end.line, x.end.column)

# if #pragma cle get to end of extended line

def main():
    index = clang.cindex.Index.create()
    tu = index.parse(sys.argv[1], args=['-x', 'c++', '-stdlib=libc++'])

    for x in tu.cursor.get_tokens():
        print x.kind
        print "  " + srcrangestr(x.extent)
        print "  '" + str(x.spelling) + "'"

if __name__ == '__main__':
    main()

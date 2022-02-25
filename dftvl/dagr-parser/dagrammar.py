#!/usr/bin/python3

#----------------------------------------------------------------------------------------------
RULES = r'''
identifier:         BACKQUOTE? ALPHA ADU*
complexid:          BACKQUOTE? ALPHA ADU* (COLONCOLON ALPHA ADU*)* (DOT ADU*)*

dagrval:            nil | bool | integer | float | string | ustring | bstring | rstring | xstring | cstring
nil:                NONE
bool:               TRUE | FALSE
integer:            MINUS? DIGIT+
float:    	    MINUS? DIGIT+ DOT DIGIT+
string:             (DQUOTE  (NONDQUOTEST|EDQUOTE)* DQUOTE) | (SQUOTE  (NONSQUOTEST|ESQUOTE)* SQUOTE)
ustring:            (UDQUOTE (NONDQUOTEST|EDQUOTE)* DQUOTE) | (USQUOTE (NONSQUOTEST|ESQUOTE)* SQUOTE)
rstring:            (RDQUOTE (NONDQUOTEST|EDQUOTE)* DQUOTE) | (RSQUOTE (NONSQUOTEST|ESQUOTE)* SQUOTE)
bstring:            (BDQUOTE (NONDQUOTEST|EDQUOTE)* DQUOTE) | (BSQUOTE (NONSQUOTEST|ESQUOTE)* SQUOTE)
xstring:            (XDQUOTE (NONDQUOTEST|EDQUOTE)* DQUOTE) | (XSQUOTE (NONSQUOTEST|ESQUOTE)* SQUOTE)
cstring:            (CDQUOTE (NONDQUOTEST|EDQUOTE)* DQUOTE) | (CSQUOTE (NONSQUOTEST|ESQUOTE)* SQUOTE)

expr:               complexid | dagrval | lst | lstref | lstcon | function | unexpr | binexpr | parexpr
binexpr:            expr (binop expr)+ 
unexpr:             unop expr 
parexpr:            LPAREN expr RPAREN
lst:                LBRACKET etuple* RBRACKET
lstref:             lst (LBRACKET [expr] RBRACKET)+
lstcon:             LBRACKET etuple FOR ituple FROM expr (WHERE expr)? RBRACKET
etuple:             expr (COMMA expr)*
ituple:             identifier (COMMA identifier)*
function:           (builtin | complexid) LPAREN (expr (COMMA expr)*)* RPAREN
builtin:            ROWS | MATCH | MATCHALL | PASS | DROP | REPLACE | INSERT | REMOVE 

unop:               NOT
binop:              AND | OR | XOR | EQ | NEQ | GEQ | GT | LEQ | LT | IN  | NIN
                    | ADD | SUB | MUL | DIV | MOD | POW

spec:               profblk pipeblk (ruleblk|ruledef|tbldef)*

profblk:            PROFILE profname LBRACE (profelt SEMI)+ RBRACE
profelt:            devprof | nsprof | glprof | improf
profname:           identifier
devprof:            DEVICE devname
nsprof:             NAMESPACE nsalias nspath
glprof:             GLOBAL gvarname
improf:             IMPORT libname
devname:            string | ustring
nspath:             string | ustring
nsalias:            string | ustring
gvarname:           identifier
libname:            identifier

pipeblk:            PIPELINE pipename LBRACE (connector SEMI)+ RBRACE
connector:          srcblk TRANSIT dstblk (PIPE pcondition)?
pipename:           identifier
srcblk:             rblkname | terminii
dstblk:             rblkname | terminii
rblkname:           identifier
terminii:           ENTRY | EXIT
pcondition:         expr

ruleblk:            BLOCK rblkname LBRACE (rulename  SEMI)+ RBRACE
rulename:           identifier

tbldef:             TABLE tblname LBRACE thdr trow+ RBRACE
thdr:               (PIPE colname)+ PIPE
trow:               (PIPE dagrval)+ PIPE
tblname:            identifier
colname:            identifier

ruledef:            RULE rulename LBRACE rexpr SEMI RBRACE
rexpr:              (letexp)* IF condition THEN action (ELSE altaction)?
letexp:             varname ASSGN expr SEMI
varname:            identifier
condition:          expr
action:             ablock
altaction:          ablock
ablock:             function | (LBRACE (function SEMI)+ RBRACE)
'''

#----------------------------------------------------------------------------------------------
TOKENS = r'''
COMMENT:            /[ \t]*--[^\r\n]*(\r|\r\n|\n)/
DELIM:              /[ \t\f\r\n]+/

BACKQUOTE:          /`/
ALPHA:              /[a-zA-Z]/
ADU:                /[a-zA-Z0-9_]/
COLONCOLON:         /::/

NONE:               /None/

TRUE:               /True/
FALSE:              /False/

MINUS:              /-/
DIGIT:              /[0-9]/
DOT:                /\./

SQUOTE:             /\'/
USQUOTE:            /u\'/
RSQUOTE:            /r\'/
BSQUOTE:            /b\'/
XSQUOTE:            /x\'/
CSQUOTE:            /c\'/
ESQUOTE:            /\\\'/
NONSQUOTEST:        /[^\'\n\r]/
DQUOTE:             /\"/
UDQUOTE:            /u\"/
RDQUOTE:            /r\"/
BDQUOTE:            /b\'/
XDQUOTE:            /x\"/
CDQUOTE:            /c\"/
EDQUOTE:            /\\\"/
NONDQUOTEST:        /[^\"\n\r]/

LBRACKET:           /\[/
RBRACKET:           /\]/
LPAREN:             /\(/
RPAREN:             /\)/
COMMA:              /,/
ASSGN:              /=/
FOR:                /for/
FROM:               /from/
WHERE:              /where/

PROFILE:            /profile/
DEVICE:             /device/
NAMESPACE:          /namespace/
GLOBAL:             /global/
IMPORT:             /import/
LBRACE:             /\{/
RBRACE:             /\}/
SEMI:               /;/
PIPELINE:           /pipeline/
ENTRY:              /entry/
EXIT:               /exit/
TRANSIT:            /=>/
PIPE:               /\|/
BLOCK:              /block/
TABLE:              /table/
RULE:               /rule/
IF:                 /if/
THEN:               /then/
ELSE:               /(else)|(otherwise)/

NOT:                /(not)|(!)/
AND:                /(and)|(&&)/
OR:                 /(or)|(\|\|)/
XOR:                /xor/
EQ:                 /(==)|((is )?equal to)/
NEQ:                /(<>)|(!=)|((is )?not equal to)/
GEQ:                /(>=)|((is )?greater than or equal to)|((is )?after or on)/
GT:                 /(>)|((is )?greater than)|((is )?after)/
LEQ:                /(<=)|((is )?less than or equal to)|((is )?before or on)/
LT:                 /(<)|((is )?less than)|((is )?before)/
ADD:                /\+/
SUB:                /\-/
MUL:                /\*/
DIV:                /\//
MOD:                /%/
POW:                /\^/
IN:                 /in/
NIN:                /not in/

ROWS:               /rows/
MATCH:              /match/
MATCHALL:           /matchall/
PASS:               /pass/
DROP:               /drop/
REPLACE:            /replace/
INSERT:             /insert/
REMOVE:             /remove/
'''

#----------------------------------------------------------------------------------------------
DIRECTIVES = r'''
%ignore COMMENT
%ignore DELIM
'''

#----------------------------------------------------------------------------------------------
DAGR_GRAMMAR = RULES + TOKENS + DIRECTIVES


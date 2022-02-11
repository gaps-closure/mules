#!/usr/bin/python3

#----------------------------------------------------------------------------------------------
RULES = r'''
identifier:         BACKQUOTE? ALPHA ADU?
complexid:          BACKQUOTE? ALPHA ADU? (COLONCOLON ALPHA ADU?)* (DOT ADU)*

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

expr:               complexid | dagrval | lst | function | unop expr | expr (binop expr)+ | LPAREN expr RPAREN
lst:                LBRACKET (expr (COMMA expr)*)* RBRACKET
function:           complexid LPAREN (expr (COMMA expr)*)* RPAREN

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
nsalias:            identifier
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
letexp:             LET varname expr SEMI
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
ADU:                /[a-zA-Z0-9_]+/
IDENT:              /[a-zA-Z][a-zA-Z0-9_]*/
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
LET:                /let/
IF:                 /if/
THEN:               /then/
ELSE:               /(else)|(otherwise)/

NOT:                /(not)|(!)/
AND:                /(and)|(&&)/
OR:                 /(or)|(\|\|)/
XOR:                /xor/
EQ:                 /(==)|((is )?equal to)/
NEQ:                /(<>)|((is )?not equal to)/
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
IN:                 /((is )?in)|(is one of)/
NIN:                /((is )?not in)|(is not one of)/
'''

#----------------------------------------------------------------------------------------------
DIRECTIVES = r'''
%ignore COMMENT
%ignore DELIM
'''

#----------------------------------------------------------------------------------------------
DAGR_GRAMMAR = RULES + TOKENS + DIRECTIVES

#----------------------------------------------------------------------------------------------
NOTUSED = r'''
propertyname:       LBRACE (NONRBRACEST)+ RBRACE
operatorname:       LBRACKET (NONRBRACKST)+ RBRACKET

ordinal:            wholenum ORDINALSUFFIX
                    | ordinalword
ordinalword:        FIRST
                    | SECOND
                    | THIRD
                    | FOURTH
                    | FIFTH
                    | SIXTH
                    | SEVENTH
                    | EIGHTH
                    | NINTH
                    | TENTH
ALL:                /all( of the)?/
ANY:                /any( of the)?/
APPLIESTO:          /applies to/
ARTICLE:            /(a)|(an)|(the)|(its)|(their)/
ARULE:              /action rule/
ASA:                /(as a)|(as an)/
ATLEAST:            /at least/
ATMOST:             /at most/
BY:                 /by/
COLON:              /:/
CONNECT:            /connect/
CONTEXT:            /context:/
CREATE:             /create/
DOCUMENT:           /document/
EACH:               /(for )?each( of the)?/
EIGHT:              /eight/
EIGHTH:             /eighth/
ELEMENTS:           /elements/
EXACTLY:            /exactly/
FIFTH:              /fifth/
FIRST:              /first/
FIVE:               /five/
FOLLOWING:          /following/
FOUR:               /four/
FOURTH:             /fourth/
FROM:               /from/
HASHAVE:            /(has)|(have)|(is)|(are)/
IFF:                /only if/
IMPLIES:            /implies/
INCOLLECTION:       /in the collection/
ISKINDOF:           /((is)|(are)) a kind of/
MATCHES:            /match(es)?/
MODEL:              /model/
NEW:                /new/
NINE:               /nine/
NINTH:              /ninth/
NO:                 /(no)|(none)/
NONRBRACEST:        /[^\}\n\r]/
NONRBRACKST:        /[^\]\n\r]/
NOTMATCHES:         /does not match/
NOTPRESENT:         /(is)|((are) not present)/
NUMBEROF:           /number of/
OF:                 /of/
ONE:                /one/
OPERATORS:          /operators/
ORDINALSUFFIX:      /(st)|(nd)|(rd)|(th)/
PRESENT:            /(is)|(are) present/
REMOVE:             /(remove)|(clear)/
REPLACE:            /replace/
REPORT:             /report/
REPRESENT:          /(represent)|(represents)/
RULESET:            /rule set/
SECOND:             /second/
SET:                /set/
SEVEN:              /seven/
SEVENTH:            /seventh/
SIX:                /six/
SIXTH:              /sixth/
SUMOF:              /sum of/
TEN:                /ten/
TENTH:              /tenth/
THAT:               /that/
THEREIS:            /there (is)|(are)/
THIRD:              /third/
THREE:              /three/
TO:                 /to/
TWO:                /two/
UNIQUE:             /unique/
USES:               /uses/
USING:              /using/
WHERE:              /where/
WITH:               /with/
'''

#----------------------------------------------------------------------------------------------

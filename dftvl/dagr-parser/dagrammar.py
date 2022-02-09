#!/usr/bin/python3

#----------------------------------------------------------------------------------------------
RULES = r'''
spec:               profblk pipeblk  (ruleblk|ruledef|tbldef)* 
profblk:            proftok profname bstart (profelt   sdelim)+ bend
pipeblk:            pipetok pipename bstart (connector sdelim)+ bend
ruleblk:            rblktok rblkname bstart (rulename  sdelim)+ bend
ruledef:            rdeftok rulename bstart (rexpr     sdelim)  bend
tbldef:             tdeftok tblname  bstart (thdr      trow+)   bend

profelt:            device dagrval | namespace nsalias nspath | global gvarname
connector:          srcblk arrow dstblk (guard condition)?
thdr:               (tdelim colname)+ tdelim 
trow:               (tdelim dagrval)+ tdelim 
rexpr:              (letexp)* cif condition cthen action (celse action)?
letexp:             let varname expr sdelim
nsalias:            identifier
devname:            squotedstring | dquotedstring
nspath:             squotedstring | dquotedstring
profname:           identifier
pipename:           identifier 
rblkname:           identifier 
rulename:           identifier
tblname:            identifier
colname:            identifier
gvarname:           identifier
varname:            identifier
srcblk:             rblkname | terminii
dstblk:             rblkname | terminii
dagrval:            nil | bool | float | integer 
                    | dquotedstring | squotedstring | unistring | bytestring | regexstring 
                    | xpathstring | cselstring 

condition:          expr
action1:             cstart (identifier | dagrval)+ cend | (identifier|dagrval)+
action:             expr

expr:               identifier
                    | dagrval
                    | function
                    | unop expr
                    | expr (binop expr)+
                    | cstart expr cend
function:           identifier cstart (argument (comma argument)*)* cend
argument:           expr

proftok:            PROFILE
pipetok:            PIPELINE
rblktok:            BLOCK
rdeftok:            RULE
tdeftok:            TABLE
terminii:           ENTRY | EXIT
device:             DEVICE
global:             GLOBAL
namespace:          NAMESPACE
guard:              PIPE
tdelim:             PIPE
bstart:             LBRACE 
sdelim:             SEMI
bend:               RBRACE
arrow:              TRANSIT
dquotedstring:      DQUOTE (NONDQUOTEST)* DQUOTE
squotedstring:      SQUOTE (NONSQUOTEST)* SQUOTE
regexstring:        REGSTART (NONSQUOTEST)* SQUOTE
xpathstring:        XPATHSTART (NONSQUOTEST)* SQUOTE
cselstring:         CSELSTART (NONSQUOTEST)* SQUOTE
unistring:          UNISTART (NONSQUOTEST)* SQUOTE
bytestring:         BYTSTART (NONSQUOTEST)* SQUOTE
nil:                NONE
integer:            MINUS? DIGIT+
float:    	    MINUS? DIGIT+ DOT DIGIT+
bool:               TRUE | FALSE
identifier:         BACKQUOTE? ALPHA ADM?
cif:                IF
cthen:              THEN
celse:              ELSE
cstart:             LPAREN
cend:               RPAREN
comma:              COMMA
unop:               NOT
binop:              EQUALS | NOTEQUALS | AND | OR
let:                LET
'''

#----------------------------------------------------------------------------------------------
TOKENS = r'''
COLON:              /:/ 
ADM:                /[a-zA-Z0-9_]+/
ALPHA:              /[a-zA-Z]/
AND:                /and/
BACKQUOTE:          /`/
BLOCK:              /block/
BYTSTART:           /b\'/
COMMA:              /,/ 
COMMENT:            /[ \t]*--[^\r\n]*(\r|\r\n|\n)/
CONNECT:            /connect/
CSELSTART:          /c\'/
DEVICE:             /device/
DELIM:              /[ \t\f\r\n]+/
DIGIT:              /[0-9]/
DOT:                /\./ 
DQUOTE:             /\"/
ELSE:               /(else)|(otherwise)/
ENTRY:              /entry/
EQUALS:             /(==)|((is )?equal to)/
EXIT:               /exit/
FALSE:              /False/
GLOBAL:             /global/
IF:                 /if/
PIPE:               /\|/
LBRACE:             /\{/
LET:                /let/
LPAREN:             /\(/ 
MINUS:              /-/ 
NAMESPACE:          /namespace/
NONDQUOTEST:        /[^\"\n\r]/
NONE:               /None/
NONSQUOTEST:        /[^\'\n\r]/
NOT:                /not/
NOTEQUALS:          /(<>)|((is )?not equal to)/
OR:                 /or/
PIPELINE:           /pipeline/
PROFILE:            /profile/
RBRACE:             /\}/
REGSTART:           /r\'/
RPAREN:             /\)/ 
RULE:               /rule/
SEMI:               /;/ 
SQUOTE:             /\'/
TABLE:              /table/
THEN:               /then/
TRANSIT:            /=>/
TRUE:               /True/
UNISTART:           /u\'/
XPATHSTART:         /x\'/
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
idstep:             (BACKQUOTE)? (ADM)+
complexid:          identifier (COLONCOLON identifier)* (DOT idstep)*

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

ADD:                /add/
AFRAG:              /action fragment/
ALL:                /all( of the)?/
ANY:                /any( of the)?/
APPLIESTO:          /applies to/
ARTICLE:            /(a)|(an)|(the)|(its)|(their)/
ARULE:              /action rule/
ASA:                /(as a)|(as an)/
ATLEAST:            /at least/
ATMOST:             /at most/
BEGIN:              /begin/
BY:                 /by/
COLON:              /:/ 
COLONCOLON:         /::/
CONTEXT:            /context:/
CREATE:             /create/
DIV:                /\// 
DOCUMENT:           /document/ 
EACH:               /(for )?each( of the)?/
EIGHT:              /eight/
EIGHTH:             /eighth/
ELEMENTS:           /elements/
END:                /end/
EXACTLY:            /exactly/
FIFTH:              /fifth/
FIRST:              /first/
FIVE:               /five/
FOLLOWING:          /following/
FOUR:               /four/
FOURTH:             /fourth/
FROM:               /from/
GREATEREQ:          /(>=)|((is )?greater than or equal to)|((is )?after or on)/
GREATER:            /(>)|((is )?greater than)|((is )?after)/
HASHAVE:            /(has)|(have)|(is)|(are)/
IFF:                /only if/
IMPLIES:            /implies/
INCOLLECTION:       /in the collection/
IN:                 /in/
ISIN:               /is one of/
ISKINDOF:           /((is)|(are)) a kind of/
ISNOTIN:            /is not one of/
LBRACKET:           /\[/
LEESEREQ:           /(<=)|((is )?less than or equal to)|((is )?before or on)/
LESSER:             /(<)|((is )?less than)|((is )?before)/
MATCHES:            /match(es)?/ 
MINUSMINUS:         /--/
MODEL:              /model/
MOD:                /mod/ 
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
PLUS:               /\+/ 
PRESENT:            /(is)|(are) present/
RBRACKET:           /\]/
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
TIMES:              /\*/ 
TO:                 /to/
TWO:                /two/
UNIQUE:             /unique/
USES:               /uses/
USING:              /using/
VFRAG:              /validation fragment/
VRULE:              /validation rule/
WHERE:              /where/
WITH:               /with/
'''

#----------------------------------------------------------------------------------------------

#!/usr/bin/python3

#----------------------------------------------------------------------------------------------
RULES = r'''
spec:               d* profblk d* pipeblk (d* (ruleblk|ruledef|tbldef))* d*
profblk:            proftok d+ profname d* bstart (d* profelt   d* sdelim)+ d* bend
pipeblk:            pipetok d+ pipename d* bstart (d* connector d* sdelim)+ d* bend
ruleblk:            rblktok d+ rblkname d* bstart (d* rulename  d* sdelim)+ d* bend
ruledef:            rdeftok d+ rulename d* bstart (d* rexpr     d* sdelim)  d* bend
tbldef:             tdeftok d+ tblname  d* bstart (d* thdr     (d* trow)+)  d* bend

profelt:            pkey d+ dagrval
connector:          srcblk d* arrow d* dstblk (d* guard d* condition)?
thdr:               tdelim (tdelim d* colname d*)+ tdelim tdelim
trow:               tdelim (tdelim d* dagrval d*)+ tdelim tdelim
profname:           identifier
pipename:           identifier 
rblkname:           identifier 
rulename:           identifier
tblname:            identifier
colname:            identifier
srcblk:             rblkname | terminii
dstblk:             rblkname | terminii
dagrval:            identifier | dquotedstring | squotedstring | integer | float | bool

rexpr:              ant sat (alt)?
ant:                cif   d* condition 
sat:                cthen d* action
alt:                celse d* action

condition:          expr 
action:             expr 
expr:               (identifier d*)+

d:                  DELIM
proftok:            PROFILE
pipetok:            PIPELINE
rblktok:            BLOCK
rdeftok:            RULE
tdeftok:            TABLE
terminii:           ENTRY | EXIT
pkey:               HARDWARE | GLOBAL 
guard:              PIPE
tdelim:             PIPE
bstart:             LBRACE 
sdelim:             SEMI
bend:               RBRACE
arrow:              TRANSIT
dquotedstring:      DQUOTE (NONDQUOTEST)* DQUOTE
squotedstring:      SQUOTE (NONSQUOTEST)* SQUOTE
integer:            MINUS? DIGIT+
float:    	    MINUS? DIGIT+ DOT DIGIT+
bool:               TRUE | FALSE
identifier:         BACKQUOTE? ALPHA ADM?
cif:                IF
cthen:              THEN
celse:              ELSE
'''

#----------------------------------------------------------------------------------------------
TOKENS = r'''
COLON:              /:/ 

ADM:                /[a-zA-Z0-9_]+/
ALPHA:              /[a-zA-Z]/
BACKQUOTE:          /`/
BLOCK:              /block/
COMMENT:            /[ \t]*--[^\r\n]*(\r|\r\n|\n)/
CONNECT:            /connect/
DELIM:              /[ \t\f\r\n]+/
DIGIT:              /[0-9]/
DOT:                /\./ 
DQUOTE:             /\"/
ELSE:               /(else)|(otherwise)/
ENTRY:              /entry/
EXIT:               /exit/
FALSE:              /false/
GLOBAL:             /global/
HARDWARE:           /hardware/
IF:                 /if/
PIPE:               /\|/
LBRACE:             /\{/
MINUS:              /-/ 
NONDQUOTEST:        /[^\"\n\r]/
NONSQUOTEST:        /[^\'\n\r]/
PIPELINE:           /pipeline/
PROFILE:            /profile/
RBRACE:             /\}/
RULE:               /rule/
SEMI:               /;/ 
SQUOTE:             /\'/
TABLE:              /table/
THEN:               /then/
TRANSIT:            /=>/
TRUE:               /true/
'''

#----------------------------------------------------------------------------------------------
DIRECTIVES = r'''
%ignore COMMENT
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
AND:                /and/
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
COMMA:              /,/ 
CONTEXT:            /context:/
CREATE:             /create/
DIV:                /\// 
EACH:               /(for )?each( of the)?/
EIGHT:              /eight/
EIGHTH:             /eighth/
ELEMENTS:           /elements/
END:                /end/
EQUALS:             /(=)|((is )?equal to)/
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
LPAREN:             /(/ 
MINUSMINUS:         /--/
MODEL:              /model/
MOD:                /mod/ 
NEW:                /new/
NINE:               /nine/
NINTH:              /ninth/
NO:                 /(no)|(none)/
NONRBRACEST:        /[^\}\n\r]/
NONRBRACKST:        /[^\]\n\r]/
NOTEQUALS:          /(<>)|((is )?not equal to)/
NOTPRESENT:         /(is)|((are) not present)/
NUMBEROF:           /number of/
OF:                 /of/
ONE:                /one/
OPERATORS:          /operators/
ORDINALSUFFIX:      /(st)|(nd)|(rd)|(th)/
OR:                 /or/
PLUS:               /\+/ 
PRESENT:            /(is)|(are) present/
RBRACKET:           /\]/
REMOVE:             /(remove)|(clear)/
REPLACE:            /replace/
REPORT:             /report/
REPRESENT:          /(represent)|(represents)/
RPAREN:             /)/ 
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

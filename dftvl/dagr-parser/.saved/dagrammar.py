#!/usr/bin/python3

#----------------------------------------------------------------------------------------------
RULES = r'''
specification:      d* profblk d* pipeblk (d* (ruleblk|ruledef))* d*
?d:                 DELIM
profblk:            PROFILE  d+ profname d* bstart (d* profelt   d* sdelim)+ d* bend
pipeblk:            PIPELINE d+ pipename d* bstart (d* connector d* sdelim)+ d* bend
ruleblk:            BLOCK    d+ rblkname d* bstart (d* rulename  d* sdelim)+ d* bend
ruledef:            RULE     d+ rulename d* bstart (d* rexpr     d* sdelim)  d* bend
profelt:            pkey d+ identifier
connector:          rblkname d* arrow d* rblkname (d* guard d* condition)?

condition:          expr 
action:             expr 
dtable:             expr

rexpr:              expr
expr:               (identifier d*)+

profname:           identifier
pipename:           identifier 
rblkname:           identifier | ENTRY | EXIT
rulename:           identifier
pkey:               HARDWARE | GLOBAL 
guard:              PIPE
bstart:             LBRACE 
sdelim:             SEMI
bend:               RBRACE
arrow:              TRANSIT
identifier:         BACKQUOTE? ALPHA ADM?
'''

#----------------------------------------------------------------------------------------------
TOKENS = r'''
ADM:                /[a-zA-Z0-9_]+/
ALPHA:              /[a-zA-Z]/
BACKQUOTE:          /`/
COMMENT:            /[ \t]*--[^\r\n]*(\r|\r\n|\n)/
CONNECT:            /connect/
DELIM:              /[ \t\f\r\n]+/
ENTRY:              /entry/
EXIT:               /exit/
GLOBAL:             /global/
HARDWARE:           /hardware/
PIPE:               /\|/
LBRACE:             /\{/
PIPELINE:           /pipeline/
PROFILE:            /profile/
RBRACE:             /\}/
RULE:               /rule/
BLOCK:              /block/
SEMI:               /;/ 
TRANSIT:            /=>/
'''

#----------------------------------------------------------------------------------------------
DIRECTIVES = r'''
%ignore COMMENT
'''

#----------------------------------------------------------------------------------------------
EXTRAS = '''
idstep:             (BACKQUOTE)? (ADM)+
complexid:          identifier (COLONCOLON identifier)* (DOT idstep)*
propertyname:       LBRACE (NONRBRACEST)+ RBRACE
operatorname:       LBRACKET (NONRBRACKST)+ RBRACKET

dquotedstring:      DQUOTE (NONDQUOTEST)* DQUOTE
squotedstring:      SQUOTE (NONSQUOTEST)* SQUOTE
wholenum:           (DIGIT)+
floatnum:    	    (DIGIT)+ DOT (DIGIT)*
boolean:            TRUE
                    | FALSE
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
DIGIT:              /[0-9]/
DIV:                /\// 
DOT:                /\./ 
DQUOTE:             /\"/
EACH:               /(for )?each( of the)?/
EIGHT:              /eight/
EIGHTH:             /eighth/
ELEMENTS:           /elements/
ELSE:               /(else)|(otherwise)/
END:                /end/
EQUALS:             /(=)|((is )?equal to)/
EXACTLY:            /exactly/
FALSE:              /false/
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
IF:                 /if/
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
MINUS:              /-/ 
MINUSMINUS:         /--/
MODEL:              /model/
MOD:                /mod/ 
NEW:                /new/
NINE:               /nine/
NINTH:              /ninth/
NONDQUOTEST:        /[^\"\n\r]/
NO:                 /(no)|(none)/
NONRBRACEST:        /[^\}\n\r]/
NONRBRACKST:        /[^\]\n\r]/
NONSQUOTEST:        /[^\'\n\r]/
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
SQUOTE:             /\'/
SUMOF:              /sum of/
TEN:                /ten/
TENTH:              /tenth/
THAT:               /that/
THEN:               /then/
THEREIS:            /there (is)|(are)/
THIRD:              /third/
THREE:              /three/
TIMES:              /\*/ 
TO:                 /to/
TRUE:               /true/
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
DAGR_GRAMMAR = RULES + TOKENS + DIRECTIVES
#----------------------------------------------------------------------------------------------

# DAGR for DFTVL

## Introduction

DFDL-Aware Guard Rules (DAGR) is a language for specifying pipelines and rules
for data validation, filtering, and transformation. The input data is expected
to be described using DFDL or XML schemata. DAGR is being developed within the
DARPA Guaranteed Architecture for Physical Security program, and in
collaboration with the Data Format Transform and Validation Language (DFTVL)
Working Group.  A future version of the DAGR draft specification may be
submitted as a candidate to the DFTVL working group.  DAGR is currently under
active development and is expected to undergo significant evolution.

DAGR is intended for use in application domains with:

   1. Strongly isolated data processing enclaves (parties)
   2. Constraints on what information can be shared between parties
   3. Enforcement points, to be configured using DAGR, that mediate the transfer between parties

While the primary use case comes from cross-domain systems, other use cases can
be found in: (i) medical records in healthcare, (ii) isolating safety-critical
control networks of a system (e.g., vetronics, avionics) car) from a less
safety-critical infotainment network, (iii) isolating the information
technology (IT) network from the operations technology (OT) for critical
infrastructure protection; (iv) isolating protected intellectual property from
the competition in legal patent dispute cases, and others.

## Requirements and Desiderata for DTVFL candidate languages

* The language is intended for non-computer-scientists; state-of-the-art
vendor-specific solutions are often based on XSLT/XPath and are complex 
* The language should allow changed to the security rules/policy in the
field, without requiring changes to hardware/firmware
* The language should support validation as well as transformation
* It is desired that the investment made by the CDS community to describe data
formats in DFDL is leveraged, so the language should support XSD and DFDL
* A declarative specification language is envisioned, not a general-purpose 
programming language
* The language should be decidable, not Turing-Complete; it should be amenable 
to formal systems analyses to support validation, verification, and accreditation
* The language must allow autogenerate of code (e.g., C, HDL, other)
and configurations to be loaded into the target hardware guards
* The human-friendly rule specifications language can be converted to a more
efficient lower-level runtime representation 
* Complex domain-speicfic data transforms (such as image processing filters) 
are not intended to be specified or implemented in the langauge, however, these
can be called through a foreign-function interface
* The input and output data resulting from the transform must conform to the
same schema; new or additional output application data units cannot be
generated from the input, however, intermediate representations during 
processing can use a different representation for computational efficiency
* It is expected that the pipelines will include the following types of transforms
  -- allow  [identity transform]
  -- block  [discard the ADU]
  -- redact [which includes drop optional element(s) in the data, or change
     the value of element(s) in the data]
* It is desired that the syntactic and semantic validation of input and
output data be modularly separated from the transform blocks
* It is being debated whether optional element(s) can be added to the data
by the transforms (e.g., to mark provenance or to flag that a transform was applied)
* The applied transforms can be contingent on header and payload fields of the
data, context variables provided out of band (e.g., OCONUS location), and state
variables set by earlier processing steps on the data
* Various hardware profiles--from simple fast hardware that may require fixed formats
with low-level rules (specified using offset,length,mask tuples) to those that can
parse and transform complex data formats--should be supported.

## Design Sketch

* Organized modularly into a pipeline of named rule blocks, which form a finite directed acyclic graph
* Each rule block has named rules that are decidable (perhaps also compose into a DAG)
* Rules can call standardized predicates and functions from an external library that are pre-compiled,
  verified, and signed
* Each rule has a condition (match) part and an action (apply) part
* Variables can be declared scope within a rule, within a rule block, or within a pipeline
* Structured English-like syntax similar to [Natural Rule Language](http://nrl.sourceforge.net)
* Initial implementation using Python Lark parsing library

We may develop a verifiable implementation in
[Idris](https://www.idris-lang.org) so that formal properties about the type
system and decidability can be investigated, while generating code in C or
Javascript. This is future work.

## DAGR Prototype HOWTO

The prototype grammar in Lark notation is in `dagrammar.py` and the prototype parser sketch is in
`dagr_parser.py`. Sample DAGR specifications can be found in the `examples` directory.

Our development environment is Ubuntu Linux 20.04 LTS with Python 3.8.10 and pip 20.3.3 installed. Here are steps to install and use DAGR.

Install the lark parser library.
```
sudo -H pip3 install lark-parser==0.11.1
```
Install GHC and Idris optionally; these may be used in the future to develop a verified version of the DAGR parser.
```
sudo apt install ghc libgmp-dev cabal-install
cabal update
cabal install idris
```
Install the 0MQ library as below and then build and install the `zc` utility from 
[https://github.com/hdhaussy/zc/](https://github.com/hdhaussy/zc/)

```
sudo apt install libzmq3-dev
```
To test the parser and a generated Python engine, open three terminals. In the first terminal, generate Python code from a DFDL and DAGR specification, and then run the generated engine.
```
python3 dagr_parser.py -v 1 -s examples/gmabw-simple.dfdl.xsd -d examples/one.dagr 
PYTHONPATH=. python3 gen.py
```
In the second terminal, listen to the output over 0MQ to the engine.
```
zc -n1 SUB ipc:///tmp/dagr_out
```
In the third terminal, send a sample input infoset over 0MQ to the engine.
```
cat examples/bw_write_221.infoset | zc -n1 PUB ipc:///tmp/dagr_in 
```

## DAGR Syntax

DAGR syntax is specified using Lark parser grammar. Whitespace is not
significant and is ignored, making the grammar easier for a LALR(1) parser.
Each comment starts with '--' and ends at the newline. Comments are ignored. A
partial excerpt of the grammar is provided here; for any missing rules (lower
case) or tokens (all upper case), consult the `dagrammar.py` file.

For symbols, DAGR allows simple identifiers `identifier` and complex
identifiers `complexid`. The optional backquote is used to allow reuse of DAGR
keywords as identifiers. Identifiers that are introduced using a let expression
in a rule definition have local scope. Column names in a table have local
scope but can be accessed globally using a complex identifier that includes the
table name. All other simple and complex identifiers have global scope.
```
identifier:         BACKQUOTE? ALPHA ADU?
complexid:          BACKQUOTE? ALPHA ADU? (COLONCOLON ALPHA ADU?)* (DOT ADU)*
```

The following primitive value types `dagrval` are recognized in DAGR and are
similar to those in Python3. These include `nil`, `bool`, `integer`, `float`,
`string` (both single and double quoted versions). For targets where Unicode
may not be default, we allow `ustring` similar to Python2. Regular expressions
`rstring` and byte strings `bstring` also follow Python3. We add two more types
to encode XPath `xstring` and  CSS selectors `cstring`. We will extend to
cover the simple types supported in DFDL. We may restrict the XPath expression 
subset (e.g., as in the Python3 XMLElementTree library). We may restrict the 
CSS selector and exclude complex selectors such as ':match'.
```
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
```

The syntax for DAGR expressions `expr` is below. In addition to identifiers and
primitives, it include lists `lst`, multi-dimensional list references `lstref`,
functions, expressions formed using binary or unary operators, and grouping
using parentheses. Functions can be builtin or foreign (through library
import). Function body definitions are not part of the language at present.
```
expr:               complexid | dagrval | lst | lstref | function | unop expr | expr (binop expr)+ | LPAREN expr RPAREN
lst:                LBRACKET (expr (COMMA expr)*)* RBRACKET
lstref:             lst (LBRACKET [expr] RBRACKET)+
function:           (builtin | complexid) LPAREN (expr (COMMA expr)*)* RPAREN
builtin:            MATCH | PASS | DROP | REPLACE | INSERT | REMOVE 
```

All action functiosn return the data item (in internal representation) or None. The builtin functions suitable for the actions are:
```
pass() -- pass the data as is
drop() -- drop the data
replace([path-list],[value-list]) -- replace the value of elements in the path list with corresponding entries in value list
remove([path-list]) -- remove the elements in the path-list
insert([path-list], [value-list]) -- insert a new element for each entry in the path list with values from the corresponding entries in the value list
```

The builtin functions for the condition is:
```
match(xstring | cstring) -- returns a list of elements in the data that match the path
```
Additional string functions including regex matching are to be defined.

Currently we have included the following unary `unop` and binary operators
`binop` for use in expressions. The latter includes logical, comparison,
arithmetic, and memebership operators.
```
unop:               NOT
binop:              AND | OR | XOR | EQ | NEQ | GEQ | GT | LEQ | LT | IN  | NIN
                    | ADD | SUB | MUL | DIV | MOD | POW

```

Each DAGR specification `spec` includes a profile definition `profblk`, a
pipeline defintion `pipeblk`, and then zero or more definitions of rule block
`ruleblk`, rule `ruledef`, and table `tbldef`.  
```
spec:               profblk  pipeblk  (ruleblk|ruledef|tbldef)*
```

A profile block has the syntax below. The profile block can have profile
elements `profelt`, which can be a device declaration to specify the target
platform profile, namespace alias declarations, DAGR library imports, and
global variable declarations. For now, the global variables are well-known
parameters that are supported by the target platform (for providing contextual
information such as the operational location). User-defined global variables
may be considered in the future. 
```
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
```

A pipeline definition `pipeblk` has the syntax below. The `pipename` is an
identifier which names the pipeline. The `rblkname` is an identifier which must
match the name of a rule block defined in the specification. In addition there
are two pre-defined special terminal blocks for entry and exit. The pipeline
sets up a number of connectors between `srcblk` and `dstblk`, which are
identifiers matching the name of a rule block or a terminal block. A pipe
condition `pcondition` can be optionally specified, and the condition
expression syntax will be defined later. If a source block connects to multiple
destination blocks, then control will transition to the destination block based
on the first connector whose guard condition expression evaluates to true. If
no connectors match, then the data is dropped. The pipeline must form a
directed acylic graph.
```
pipeblk:            PIPELINE pipename LBRACE (connector SEMI)+ RBRACE
connector:          srcblk TRANSIT dstblk (PIPE pcondition)?
pipename:           identifier
srcblk:             rblkname | terminii
dstblk:             rblkname | terminii
rblkname:           identifier
terminii:           ENTRY | EXIT
pcondition:         expr
```

A rule block `ruleblk` has the syntax below. The `rblkname` is an identifier
which can be referenced in the pipeline defintion. The rule block is simply a
sequence of `rulename` entries, which are identifiers that match the name of a
rule defined in the specification. A rule can be referenced in multiple rule
blocks.
```
ruleblk:            BLOCK rblkname LBRACE (rulename  SEMI)+ RBRACE
rulename:           identifier
```

A table definition has the syntax below. The name of the table `tblname` is an
identifier that can be referenced in other rules. The table has a header `thdr`
and one or more rows `trow`. The table uses '|' to delimit cells in a header or
row. For stylistic purposes, comment lines beginning with '--' such as
'--+------+------' can be used before and after a header or row; these are
ignored. Each cell in the header contains a column name `colname` which is an
identifier. Each cell in a row can contain a value of any primitive type
(`dagrval`).
```
tbldef:             TABLE tblname LBRACE thdr trow+ RBRACE
thdr:               (PIPE colname)+ PIPE
trow:               (PIPE dagrval)+ PIPE
tblname:            identifier
colname:            identifier
```

A rule definition has the syntax below. The rule has a name `rulename` which is
an identifier which can be referenced in rule blocks. The rule expression
`rexpr` includes one or more let expressions that declare a variable `varname`
which is an identifier and assign an expression to it. The variable is locally
scoped within the rule definition.  It can shadow a global identifier of the
same name.  The main rule expression has an antecendent `condition`, an
`action` and an optional `altaction`. If the `condition` evaluates to True,
then the `action` is evaluated, otherwise the optional `altaction` expression
is evaluated if specified. As with most languages, the evaluation of the
`action` expressions is lazy, and done only as needed after the evaluation of
the `comdition` expression. The `condition` expression cannot change the data
being processed, but can keep state in the local variables. The `action` and
`altaction` are blocks with a sequence of function invocation statements each 
ending with a semicolon. Action functions can pass or drop the data, and can 
modify it. Each function in the action block must return either the transformed 
data or `None`.
```
ruledef:            RULE rulename LBRACE rexpr SEMI RBRACE
rexpr:              (letexp)* IF condition THEN action (ELSE altaction)?
letexp:             LET varname expr SEMI
varname:            identifier
condition:          expr
action:             ablock
altaction:          ablock
ablock:             function | (LBRACE (function SEMI)+ RBRACE)
```

A note on sharing state. Currently we have vaiables only at two levels of
scoping, namely, local to a rule definition or global.  There are two other
mechanisms available for sharing runtime state: (i) Rules may set an attribute in
an element and pass it to downstream elements to read and or modify, and these
attributes will need to be removed at exit; (ii) foreign functions may allow
state to be set in static variables.

A note on typing. The current plan is to keep DAGR free of explicit type 
declarations and have them flow in from the DFDL.

## TODO

* Identify test use cases, flesh out expr/ablock syntax and useful functions to implement
* Load DFDL map and do type checking and validation
* Idris formal model to establish decidability 


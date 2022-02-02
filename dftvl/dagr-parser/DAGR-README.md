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
  3. Enforcement points, to be configured using DAGR, that mediate the transfer
     between parties

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
python3 dagr_parser.py -v 1 -s examples/gmabw.dfdl.xsd -d examples/one.dagr 
python3 gen.py
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

Each DAGR specification includes a profile definition `profblk`, a pipeline defintion `pipeblk`, 
and then zero or more definitions of rule block `ruleblk`, rule `ruledef`, and table `tbldef`.

Whitespace is not significant and is ignored, making the grammar easier for a LALR parser.
A comment starts with '--' and ends at the newline. Comments are ignored. 

The following primitive value types `dagrval` are recognized in DAGR:
 * `nil`        null, same to python3 `None`
 * `integer`    integers, same as python3 int
 * `float`      floating point numbers, same as python3 float
 * `bool`       boolean values, same as python3 bool
 * `string`     strings, same as python3 string, includes single- and double-quoted variants
 * `unicode`    unicode strings, same as python3 u'...' 
 * `bytestring` byte sequences, same as python3 b'...' 
 * `regex`      regular expressions, same as python3 r'...' 
 * `cselector`  c'...' containing a CSS selector in the string
 * `xselector`  x'...' containing a XPath on3 r'' regular expression strings
 * `identifier` a symbol, with the following pattern `\`?[a-zA-Z][a-zA-Z0-9_]+`, 
   with the backquote used to escape DAGR keywords when used as identifiers


Identifiers are globally unique with teo exceptions: identifiers introduced using a let
expression in a rule definition (to be described later) and column names in a table (to
be defined later). They have local scope, and in the case of the table, they can be
refernced as `tblname`.`colname`.

Lists, tuples, and dictionaries may be defined in the future.

A profile block has the following syntax:
```
profblk:            'profile' profname '{' (profelt ';')+ '}'
profname:           identifier
profelt:            'device' identifier | 'namespace' identifier string | 'global' identifier
```

The profile block can have profile elements `profelt`, which can be a device declaration to 
specify the target platform profile, zero or more namespace shorthand declarations, and zero 
or more global variable declarations. For now, the global variables are well-known parameters
that are supported by the target platform (for providing contextual information such as the
operational location). User-defined global variables may be considered in the future.

A pipeline definition `pipeblk` has the following syntax:

```
pipeblk:            'pipeline' pipename '{' (connector ';')+ '}'
connector:          srcblk '=>' dstblk ('|' condition)?
srcblk:             rblkname | terminii
dstblk:             rblkname | terminii
terminii:           'entry' | 'exit'
```

The `pipename` is an identifier which names the pipeline. The `rblkname` is an
identifier which must match the name of a rule block defined in the
specification. In addition there are two special terminal blocks named 'entry'
and 'exit' that are available (and cannot be defined). The pipeline sets up a
number of connectors between `srcblk` and `dstblk`, which are identifiers
matching the name of a rule block or a terminal block. A guard condition can be
optionally specified, and the condition expression syntax will be defined
later. If a source block connects to multiple destination blocks, then control
will transition to the destination block based on the first connector whose
guard condition expression evaluates to true. The pipeline must form a directed
acylic graph.

A `ruleblk` has the following syntax:
```
ruleblk:            'block' rblkname '{' (rulename  ';')+ '}'
```
The `rblkname` is an identifier which can be referenced in the pipeline defintion. The rule block 
is simply a sequence of `rulename` entries, which are identifiers that match the name of a rule 
defined in the specification. A rule can be referenced in multiple rule blocks.

A table definition has the following syntax:

```
tbldef:             'table` tblname  '{' thdr trow+  '}'
thdr:               ('|' colname)+ '|'
trow:               ('|' dagrval)+ '|'
```
The name of the table `tblname` is an identifier that can be referenced in other rules. The table has a header `thdr` and one or more rows `trow`. The table uses '|' to delimit cells in a header or row. For
stylistic purposes, comment lines beginning with '--' such as '--+------+------' can be used before
and after a header or row; these are ignored. Each cell in the header contains a column name `colname` 
which is an identifier. Each cell in a row can contain a value of any primitive type (`dagrval`).

A rule definition has the following syntax:

```
ruledef:            'rule' rulename '{' rexpr ';' '}'
rexpr:              (letexp)* 'if' condition 'then' action ('else' action)?
letexp:             'let' varname dagrval ';'
```

XXX: Specify syntax for condition and action expressions here



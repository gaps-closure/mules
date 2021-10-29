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

We may develop a verifiable implementation in [Idris](https://www.idris-lang.org) so that formal properties 
about the type system and decidability can be investigated, while generating code in C or Javascript. This is
future work.

## DAGR Prototype

### Grammar and Parser

The prototype grammar in Lark notation is in `dagrammar.py` and the prototype parser sketch is in
`dagr_parser.py`. Sample DAGR specifications can be found in the `examples` directory.

Each DAGR specification must include a profile block first which specified the hardware profile and
available our of band global configuration variables supported by the hardware.

It must be followed by a pipeline block. which has a list of directed links between rule blocks.
Two terminii `entry` and `exit` are mandatory rule blocks. The connections can optionally include 
a guard condition which determines whether an outgoing edge is to be taken. The pipeline must
form a directed acylic graph, and except the terminii, all referenced rule blocks must be defined
in the specification.

Comments in the specification start with `--` and everything to the end of line is ignored.
Whitespace is not significant except to serve as token delimiters for parsing.

The rest of the specification can contain one of more named rule blocks, named rule definitions, and 
named table definitions in any order. Each rule block is simply a sequence of rule names.

Types, variable scoping, and expression syntax will be described here in the future.

### HOWTO

Our development environment is Ubuntu Linux 20.04 LTS with Python 3.8.10 and pip 20.3.3 installed.

Install the lark parser library.
```
sudo -H pip3 install lark-parser==0.11.1
```

Optionally, install GHC and Idris.

```
sudo apt install ghc
sudo apt install libgmp-dev
sudo apt install cabal-install
cabal update
cabal install idris
```


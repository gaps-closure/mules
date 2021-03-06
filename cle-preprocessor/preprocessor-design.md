# Requirements/Scope
The CLOSURE pre-processor is a source transformer that will:
* take C/C++ programs annotated with CLE pragmas as input (these pragmas are toolchain-neutral)
* generate C/C++ programs annotated with toolchain-specific source annotations, specifically new LLVM __attribute__ annotations which we define for the project
* additionally generate a file containing mappings from each annotation label to the corresponding CLE-JSON specification which contains additional detail (such as downgrade hints) to be used by downstream CLOSURE tools

The output C/C++ of the pre-processor will go to a minimally modified LLVM clang that will support the CLOSURE-specific LLVM __attribute__ annotations and pass them down to the LLVM IR level.

The pre-processor requires a C/C++ source parser (but not a full-blown compiler) so that applicable functions, variables, structs, classes, and other language elements can be identified and annotated appropriately based on the pragma which may be specified for the next non-empty, non-comment line or for an entire block of code.  For this purpose, the parser could borrow C/C++ grammar and parsing code from another project ([see notes here](http://www.nobugs.org/developer/parsingcpp/)), or leverage code from an exiting toolchain (e.g., gcc, clang) if needed. 

The ICD for the pre-processor (which we will specify/refine as a team) will be the CLE specification and new annotations attributes we define along with where they need to be placed.  As a starting point, we have a [draft CLE spec](https://github.com/gaps-closure/cle-spec/blob/master/specification.md), and a toy example (we will need to add support for code blocks, structs, functions, classes, et cetera).

Intial source is C file containing:
```
int * secretvar = 0;
```
Developer annotates the C file as follows:
```
#pragma cle def HIGHONE { //CLE-JSON, possibly \-escaped multi-line with whole bunch of constraints } #pragma cle HIGHONE int * secretvar = 0;
```
After running the preprocessor, we should get a C file with pragmas removed but with __attribute__ inserted (in all applicable places), e.g.,:
```
int * __attribute__(type_annotate("HIGHONE")) secretvar = 0;
```
We should also get a .hints file containing:
```
[{"label": 'HIGHONE', "cle-json": {...}]
```
Eventually, as we evolve CLE (e.g, more language elements can be annotated and more MLS partitioning hints/detail provided), add support for more source languages (e.g., C, C++, Golang, ObjectiveC), and possibly add more toolchains (e.g., LLVM, gcc), we will need to enhance the pre-processor functionality.

# Implementation Choices:
Rather than use a custom grammar/parser, I recommend we use libclang which is well aligned with the rest of our system and is more likely to track language changes better than a custom grammar/parser. The following links are quite interesting:
* Doing it in Python+libclang [python+clang](https://eli.thegreenplace.net/2011/07/03/parsing-c-in-python-with-clang)
* Doing it in C++ using cppast+libclang [CPPAST](https://github.com/foonathan/cppast)

The pythonic route may be quicker for an initial prototype of the pre-processor. Along those lines, please see this [code generator example](http://szelei.me/code-generator).  However, the python bindings are somewhat limited and C/C++ route may be necessary.

This [link](https://blog.quarkslab.com/implementing-a-custom-directive-handler-in-clang.html) describes how to implement a custom pragma directive and annotations in clang.

For most of the features, we need, the problem could be greatly simplified by leveraging [pragma clang attribute](https://clang.llvm.org/docs/LanguageExtensions.html#specifying-an-attribute-for-multiple-declarations-pragma-clang-attribute)

This pragma already supports blocks with push/pop, has powerful subject match rules applying selectively to functions, variables, fields, etc.  Looks like many of the things I had spec'd in CLE as useful features could be readily mapped to "#pragma clang attribute" avoiding tedious parsing and understanding of C/C++ AST.

Note that pragma clang attribute does not have a apply-only-to-next line mode, it requires a separate push/pop for each attribute, and does not currently support type_annotate (we will have to add that support). 

We will still keep "#pragma cle" to keep our work toolchain-neutral, but we could more closely align the CLE spec to pragma clang attribute to save implementation effort massively! So we will still need the pre-processor, which will
  * Snarf the CLE label to JSON mappings and write to companion file
  * Rewrite the pragma cle to pragma clang directives for most of the functionality we need

Now that we have reduced the problem to the above two, we can set up a Lark grammar for CLE and use python clang bindings to do the tokenizing of the C/C++ program. We now have a toy program to do this.

Another interesting alternative is to write a m4 program, after all, m4 is designed for pre-preprocessing tasks. The downside of this approach, however, is that unlike a custom parser, we may not be able to check all CLE and CLE-JSON spec and provide user with helpful debugging comments.


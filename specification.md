# CLE Specification and Usage
1. CLE for C/C++ is based on pre-processor directives; a GAPS-unaware compiler can ignore the directives and compile the code (but without MLS constraints)
2.	CLE directives have the following syntax:  
   ```
   #pragma cle def LABEL CLE-JSON // assign a label to a CLE JSON object  
   #pragma cle LABEL              // apply to the next statement  
   #pragma cle begin LABEL        // open a block and apply to the block  
   #pragma cle end LABEL          // close the block
   ```
3.	`LABEL` is an arbitrary user-declared symbol using ASCII characters with pattern `[a-z][A-Za-z0-9_]*` 
4.	`CLE-JSON` is a JSON document with syntax described later
5. Directives can apply to the next statement or to a block if usign begin/end
6.	Directives (either next statement or block) can be applied to variable, record (struct, class, union), field of record, struct or class field, type alias (typedef), function, or enum
7.	Comments and empty lines can occur between the pragma directive and the succeeding statement and also within blocks
8.	A block is opened and closed using the following syntax:  
   ```
   #pragma cle begin LABEL CLE-JSON  
   [C/C++ statements and/or functions]  
   #pragma cle end LABEL  
   ```  
9.	Blocks can be nested to arbitrary depth
10.	The `CLE-JSON` at the beginning of a block applies to the entire block unless overridden by an inner block
11.	`CLE-JSON` is a JSON document with the following structure:
<pre><code>{
 <b>“level”:</b>  &ltstring>,                        // any one of multiple security levels specified in GSPS file with total ordering
 <b>“markcode”:</b> &ltBoolean>,                     // annotation applies to code/computation if True, else only to data optional, useful if function/algorithm is classified at specified level
 <b>“types”:</b> &lt[string]>,                       // top-level annotation applies to all variables with specified user-defined types or classes 
 <b>“target”:</b> &lttarget-spec>,                   // optionally a target architecture can be specified for a block or function  
 <b>“threads”:</b> &ltint>,                          // maximum number of threads, block or function can be split, 1 to prevent
 <b>“procs”:</b> &ltint>,                            // maximum number of processes, block or function can be split, 1 to prevent
 <b>“optimize”:</b> &ltoptimize-parameters>,         // minimize cross-domain flow by caching/memoization or proactive pub/sub on change of a variable
 <b>“cdf”:</b>                                     // cross domain flow restrictions 
  [ // can specify restrictions relative to a set of peer levels, can have multiple such blocks separate blocks for ingress and egress         
    {
    <b>“remotelevel”:</b> [>|<|>=|<=|==] &ltstring>, // any security level specified in GSPS file
    <b>“ratelimit”:</b> &ltrate-spec>,               // maximum datarate
    <b>“direction”:</b> “ingress” | “egress”,
    <b>“ipcstyle”:</b> &lt[string]>,                 // any available IPC style specified in GSPS file
    <b>“bus”:</b> &lt[string]>,                      // any type of interconnection specified in GSPS file 
    <b>“guardhint”:</b>  
      {
      <b>“oneway”:</b> &ltBoolean>,
      <b>“downgrade”:</b> &ltdowngrade-params>,
      <b>“upgrade”:</b> &ltupgrade-params>,
      <b>“crypto”:</b> &ltcrypt-params>,
      <b>“encode”:</b> &ltencode-params>             // encoding including compression            
      }
    }
  ]
}
</code></pre>
12.	The Global Security Policy Specification (GSPS) format will be specified in a separate document.  For the purposes of this document,  we treat that a fixed set of pre-defined allowed values and defaults are made available by the GSPS.
13. The only mandatory field is `level`, rest are optional. For convenience, a beginner could use a minimal `CLE-JSON` that includes only the `level` parameter, with all other parameters can be determined by downstream CLOSURE tools based on defaults from the GSPS; an expert could specify more constraints and preferences.  For example, if `guardhint` is provided, the tool can automatically pull in functions from a library, else stub functions may be created for the developer to populate. 
14.	Downgrade parameters `<downgrade-params>` include:  
  `{trunc: True | round: True | precision: <int> | scrub: [<string>]}`  
15.	Other parameters are being defined, and semantics of each of these will be specified in more details

# CLE Steel Thread Feature Subset
The features that will be supported in the initial steel thread version will be identified and documented here. For example, in early versions, only block-style directives are supported, only `annotate` (not `type_annotate`) is supported, and a subset of the CLE-JSON fields are supported. 

# CLE Pre-processor Reference Implementation
The CLE is defined for C/C++ in a compiler neutral way.  It is also designed (separable CLE-JSON) so that much of the definitions can carry over to other source languages. See the [CLE Preprocessor](https://github.com/gaps-closure/cle-preprocessor) for more information on this implementation. 

Consider the following toy example:
```
#pragma cle def HIGHONE { //CLE-JSON with whole bunch of constraints } 
#pragma cle HIGHONE
int * secretvar = 0;
```  
A CLOSURE pre-processor for clang does the following:  
1. Handles the `#pragma cle ...` directives
2. Inserts an attribute into the code in a manner clang can handle  
 `int * __attribute__(type_annotate("HIGHONE")) secretvar = 0;`  
  Note:  `type_annotate` is a new feature which will allow user defined annotations via the attribute mechanism, this requires some clang modification; teh fallback is to use `annotate` instead of `type_annotate`
3. Creates a companion file with mappings, e.g., `HIGHONE` to the `CLE-JSON`

Ideally we should not have to modify clang at all; however since custom type annotations allow considerable source level type checking, we will minimally modify clang (in the style of Quala, see https://github.com/sampsyo/quala) to pass the type and function annotations we care about into the LLVM IR. This clang mod is done infrequently, but CLE and CAPO can evolve rapidly.

Since CLE is toolchain-neutral, a similar gcc based solution can be created later if absolutely necessary. CLE shall stay the same, but the manner in which annotations are passed to gcc IR will be different. Most of CLE-JSON can carry over to other source languages. Downstream Compiler, and Partitioner Optimizer (CAPO) tools then takes as input the annotated LLVM IR code and the companion file and perform further analysis, partitioning, and optimization, and the resulting programs are fed to the standard LLVM linker and target-specific backends.

# CLE Annotation Examples
See the [CLE Preprocessor](https://github.com/gaps-closure/cle-preprocessor) for some examples.

# CLE Formal Grammar
To be specified. Meanwhile, a Python Lark grammar can be found with the [CLE Preprocessor](https://github.com/gaps-closure/cle-preprocessor).


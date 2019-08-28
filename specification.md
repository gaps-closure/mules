# CLE Specification and Usage
1. CLE for C/C++ is based on pre-processor directives; a GAPS-unaware compiler can ignore the directives and compile the code (but without MLS constraints)
2.	CLE directives have the following syntax:  
   ```
   #pragma cle def JLABEL CLE-JSON // assign a label to a CLE JSON object  
   #pragma cle JLABEL|CLE-JSON  // apply either the label or the inline CLE JSON object to next statement  
   #pragma cle begin LABEL JLABEL|CLE-JSON // open a block and apply to the block  
   #pragma cle end LABEL  // close the block
   ```
3.	`LABEL` and `JLABEL` are arbitrary user-declared symbols using ASCII characters with pattern `[a-z][A-Za-z0-9_]*` 
4.	`CLE-JSON` is a JSON document with syntax described later
5.	User may optionally define a `CLE-JSON` document and assign it a user-friendly label using `def` 
6.	Directives can be top-level, on-function, on-statement, on-struct, on-field, and on-block
7.	Comments and empty lines can occur between the pragma directive and the succeeding statement, function, struct, or field, and within blocks 
8.	Top-level directives apply to the entire file, but could be restricted by the `CLE-JSON` to specific types (most useful when restricting to types)
9.	Top-level directives in header files apply only to definitions and declarations in the header file itself, and not to the importing files **[clarify, discuss and revisit]**
10.	On-struct directives apply to the next struct declaration
11.	On-field directives apply to the next field of a struct or class
12.	On-function directives apply to the next function definition  or declaration in header file
13.	On-statement directives apply to the next statement
14.	On-block directives demarcate a block of code, which may contain statements and/or functions at the top-level, or statements within a function
15.	A block is opened and closed using the following syntax:  
   ```
   #pragma cle begin LABEL CLE-JSON  
   [C/C++ statements and/or functions]  
   #pragma cle end LABEL  
   ```  
16.	Blocks cannot be defined within a statement or within a struct
17.	Blocks can be nested to arbitrary depth
18.	The `CLE-JSON` at the beginning of a block applies to the entire block unless overridden by an inner block
19.	`CLE-JSON` is a JSON document with the following structure:
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

20.	The only mandatory field is `level`, rest are optional. For convenience, a beginner could use a minimal `CLE-JSON` that includes only the `level` parameter, with all other parameters based on defaults from the GSPS; an expert could specify more constraints and preferences.  For example, if `guardhint` is provided, the tool can automatically pull in functions from a library, else stub functions may be created for the developer to populate. 
21.	For convenience the developer can define several `CLE-JSON` instances with mnemonic `JLABEL` and use them to reduce clutter within the code
22.	Downgrade parameters `<downgrade-params>` include:  
  `{trunc: True | round: True | precision: <int> | scrub: [<string>]}`  
23.	Other parameters are being defined, and semantics of each of these will be specified in more details
# CLE Steel Thread Feature Subset
The features that will be supported in the initial steel thread version will be identified and documented here.  
# CLE Pre-processor Reference Implementation
The CLE is defined for C/C++ in a compiler neutral way.  It is also designed (separable CLE-JSON) so that much of the definitions can carry over to other source languages.  
**Insert diagram of workflow here from source, CLE to annotated IR for CAPO**  
Consider the following toy example:
```
#pragma def HIGHONE { //CLE-JSON with whole bunch of constraints } 
#pragma cle HIGHONE
int * secretvar = 0;
```  
A CLOSURE pre-processor for clang does the following:  
1. Consumes the pragmas
2. Inserts an attribute into the code in a manner clang can handle  
 `int * __attribute__(type_annotate("HIGHONE")) secretvar = 0;`  
  Note:  `type_annotate` is a new feature which will allow user defined annotations via the attribute mechanism, this requires some clang modification
3. Creates a companion file with mappings, e.g., `HIGHONE` to the `CLE-JSON`, some of which may be derived from GSPS file   

Ideally we should not have to modify clang at all; unfortunately since user-defined attributes are not part of clang by default and existing attributes do not meet our needs, we will minimally modify clang (in the style of Quala, see https://github.com/sampsyo/quala) to pass the type and function annotations we care about into the LLVM IR.
This clang mod is done infrequently, but CLE and CAPO can evolve rapidly.
Since CLE is compiler-neutral, a similar gcc based solution can be created later if absolutely necessary. CLE shall stay the same, but the manner in which annotations are passed to gcc IR will be different.  Most of CLE-JSON can carry over to other source languages.)
CAPO then takes as input the annotated LLVM IR code and the companion file and does the transformations needed. Output of CAPO will then be fed to standard LLVM linker and backend.
To be refined further.
# CLE Annotation Examples
To be determined
# CLE Formal Grammar
To be determined




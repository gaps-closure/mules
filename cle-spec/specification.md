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
 <b>“level”:</b>  &ltstring>,       // [Required] any one of multiple security levels specified in GSPS file with total ordering
 <b>"$schema":</b> $lturl>,     // [Optional] URL to the schema (if used by external tools)
 <b>"$comment":</b> $ltstring>,     // [Optional] A comment to be included in the CLE (including final CLEMap)
 <b>“cdf”:</b>      // [Optional] cross domain flow restrictions
  [ // can specify restrictions relative to a set of peer levels, can have multiple such blocks separate blocks for ingress and egress
    {
    <b>“remotelevel”:</b>  &ltstring>,       // the remote (target) level, must be defind in a seperate cle entries
    <b>"direction":</b> &ltenum>,       //The direction of the data flow: one of ["egress", "ingress", "bidirectional"]
    <b>“guarddirective”:</b>        //guard rules, previously known as "guardhint"
      {
      <b>"operation":</b> &ltenum>,   // [Option] one of "allow", "block", "redact"
      <b>“oneway”:</b> &ltBoolean>,   //[Option]If the data is sent one way (true/false)
      <b>“gapstag”:</b> &ltArray>    //[Option]must be an array of 3 integers
        [ <b>mux</b>, <b>sec</b>, <b>type</b> ] &ltintegers> //3 integers >=0 representing mux, sec, type
      }
    }
    <b>“argtaints”:</b> // [Optional*] For functions, indicates function argument taints (input cle tags)
        [       //list representing one entry for each function argument (empty list is valid)
            [cle-entry, cle-entry, ...]      //list of zero or more taints (cle definitions) related to each argement
        ]
    <b>“codtaints”:</b> // [Optional*] For functions, incudes any taints (cle tags) to bless inside the block of code
        [cle-entry, cle-entry, ...]      //list of zero or more taints (cle definitions) used in the code block
    <b>“rettaints”:</b> // [Optional*] For functions, includes the returned taint (output cle tags)
        [cle-entry, cle-entry, ...]      //list of zero or more taints (cle definitions) needed by the returned value
  ]
}

* If argtaints, codtaints, or rettaints are included, all three must be incuded

</code></pre>
12.	The Global Security Policy Specification (GSPS) format will be specified in a separate document.  For the purposes of this document,  we treat that a fixed set of pre-defined allowed values and defaults are made available by the GSPS.
13. The only mandatory field is `level`, rest are optional. For convenience, a beginner could use a minimal `CLE-JSON` that includes only the `level` parameter, with all other parameters can be determined by downstream CLOSURE tools based on defaults from the GSPS; an expert could specify more constraints and preferences.  For example, if `guardhint` is provided, the tool can automatically pull in functions from a library, else stub functions may be created for the developer to populate. 
//14.	Downgrade parameters `<downgrade-params>` include:  
  `{trunc: True | round: True | precision: <int> | scrub: [<string>]}`  
15.	Other parameters are being defined, and semantics of each of these will be specified in more details

# CLE Steel Thread Feature Subset
The features that will be supported in the initial steel thread version will be identified and documented here. For example, in early versions, only block-style directives are supported, only `annotate` (not `type_annotate`) is supported, and a subset of the CLE-JSON fields are supported. 

# CLE Pre-processor Reference Implementation
The CLE is defined for C/C++ in a compiler neutral way.  It is also designed (separable CLE-JSON) so that much of the definitions can carry over to other source languages. See the [CLE Preprocessor](../cle-preprocessor) for more information on this implementation. 

Example CLE definition:
```
#pragma cle def XDLINKAGE_GET_A {"level":"orange",\
  "cdf": [\
    {"remotelevel":"purple", \
     "direction": "bidirectional", \
     "guarddirective": { "operation": "allow"}, \
     "argtaints": [], \
     "codtaints": ["ORANGE"], \
     "rettaints": ["TAG_RESPONSE_GET_A"] \
    } \
  ] }
  
```

# CLE Formal Grammar
A Python Lark grammar can be found with the [CLE Preprocessor](../cle-preprocessor).
describing the grammar of the Preproceser's interpritation of the CLE tags and definitions.

While the CLE json its self is defined by the [JSON Schema](schema/cle-schema.json).


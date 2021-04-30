# CLOSURE CLE Specification for Java

## Syntax

As with the C version, we can define or apply an annotation.

The definition of a CLE annotation connects a CLELABEL with a CLE-JSON. The CLEJSON schema is common across languages, currently Java, C/C++, and message-flow design. The `@interface CLELABEL` is used to declare a new CLE label, and the `@cledef` annotation on this interface declaration associates a CLEJSON with the CLELABEL as below. 

```
// Annotation type declaration for cledef to be moved to a common library
public @interface cledef { String json(); }

@cledef(json="{\"level\": \"orange\"}"); 
@interface ORANGE_SHAREABLE
```

The json string can either be a "stringified" JSON or multiline JSON using a text block in JDK 13 and higher.

```
// In JDK 13 and higher
@cledef(json="""
   {
     "level": "orange"
   }
   """); 
@interface ORANGE_SHAREABLE
```

This is equivalent to the `#pragma cle def CLELABEL CLEJSON' in CLE for C/C++.

A CLE annotation is a Java annotation which has been annotated with a '@cledef' and other Java annotations can co-exist.

Once a CLE annotation has been defined, it can be applied to program elements as below.

```
Class FUBAR { ... };

...
  // inside method body of some other class
  @ORANGE_SHAREABLE
  FUBAR foo;
...

```
 
When applying a CLE annotation `@CLELABEL` is analogous to `#pragma cle CLELABEL' in C/C++. In the case of Java,the next line version is supported, block annotation version (with begin and end) is not supported.

Local variables (primitives or Class instances), fields, and methods can be annotated using CLE.  Method annotations will include allowed taints on parameters and return values. 

 - Position 1: any class member that is annotated must be final
    * the annotation will be inherited by all subclass instances
    * the annotation does not apply to methods of superclasses
 - Position 2: non-final class members can be annotated, but upon overriding, the member must be re-annotated and if method, its body must be audited 
    * classes in the hierarchy in-between inherit the parent's annotation on the method
    
Position 1 seems easier to argue correctness for, but is restriuctive for the developer. To be revisited.

## Implications for partitioned code
In a correct partitioning the following must hold.

All instances of a class with an annotated member must be assigned to the corresponding enclave. All members of an instance (including inherited ones) must have the same level.

Only control-flow cut can be a XD-RPC-wrapped method invocation, and only data-flow cut can be paramters and return values for such a method invocation.   All parameters and return values involving invocation of this method must honor the allowed taints.

Taint flow needs to be defined in terms of Java System Dependency Graph or other such formalism. Except for edges in the cut, the endpoints of an edge must have the same label, and any label coercion must occur through a blessed method.




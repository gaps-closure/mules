# Placeholder for Java CLE

There are only two new annotation types in Java CLE: cle and cle-def

@interface cle {String label();}
// Applies to program elements TBD (instances, fields, methods)

How to associate json={...} with the annotation instance @cle(label="FOO")?

public @interface cledef { String json(); }

@cledef(json="{}")   // CLE-JSON or 1-to-1 equivalent annotation contains level, cdfs, taints etc. 
@interface ORANGE_SHAREABLE  

// Developer annotates thus ...

Class FUBAR;

@ORANGE_SHAREABLE
FUBAR foo;

What can we annotate?
  -- class instances
  -- fields
  -- methods

What are the rules for inheritance and polymorphism

To be correct in the partitioned code
  All fields in an instance must have the same label as that of the instance (or must be coerced through a blessed method)
  All methods in an instance must have the same level as that of the instance
  All parameters and return values involving invocation of this method must honor the allowed taints
  Any label coercion must occur through a blessed method
  An annotated method cannot exist in any instance (of any class) in another enclave



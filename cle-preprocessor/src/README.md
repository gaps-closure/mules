Contains example C/C++ code and python scripts that use python bindings to 
parse the code using python bindings to libclang. Examples that use 
clang.cindex Python bindings to get the AST and the tokens are in 
cindex-dump.py, showast.py, and simparse.py. 

Going further, a prototype CLE preprocessor using python Lark and libclang 
is provided in qd_cle_preprocessor.py. We assume a system with CLOSURE LLVM
(with quala type annotation) installed and PDG has been built per 
[instructions](https://githib.com/gaps-closure/team-instructions). Install 
python lark-parser package as follows.
```
sudo -H pip3 install lark-parser
```

Also required is the jsonschema python module (3.2.0 or newer) in order to
validate the CLE with the json schema
```
sudo -H pip3 install jsonschema -U
```

A typical flow of how the CLE preprocessor will be used by a developer
is as follows:
* Developer creates a C/C++ program `helloworldmars.cpp`
* Developer uses a text editor and annotates the C/C++ progam with `#pragma cle ...` directives as shown in `helloworldmars-cle.cpp`
* Developer (or build script) runs the preprocessor on the annotated program
```
./qd_cle_preprocessor.py -f helloworldmars-cle.cpp
```
* The preprocessor extracts the CLE-LABEL to CLE-JSON mapppings in a companion file `helloworldmars-cle.cpp.clemap.json` for use by downstream CLOSURE tools, and a `helloworldmars-cle.mod.cpp` which contains clang-specific pragmas
and annotations.
* Note: the preprocessor can be used with the `-a type` to generate type annotations instead of clang native annotations. However full support for type annotation is not completed for clang, and in the interim, the user will have to manually annotate the code with `__attribute__((type_annotate("foo")))` at every applicable declaration. For example, see `helloworldmars-clangtypann.cpp` and `sample-typeannot.cpp` to see examples of type annotation.
* Compile and run the code
```
clang++ -stdlib=libc++ helloworldmars-cle.mod.cpp
```
* Run only the front-end to generate LLVM IR `helloworld-cle.mod.ll`
```
clang++ -stdlib=libc++ -S -emit-llvm helloworldmars-cle.mod.cpp
```
* Generate LLVM IR Bitcode helloworldmars-cle.mod.bc
```
llvm-as helloworldmars-cle.mod.ll
```
* Generate PSU Program Dependency Graph to generate `pdgragh.main.dot`
```
opt -load ~/gaps/top-level/pdg/build/libpdg.so -dot-pdg helloworldmars-cle.mod.bc
```
* View the PDG
```
xdot pdgragh.main.dot
```


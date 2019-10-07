Contains example code using python bindings to clang for developing 
a CLE preprocessor. We assume a system with CLOSURE LLVM (with quala 
type annotation) installed.

Examples use of clang.cindex Python bindings to get the AST and the 
tokens are in cindex-dump.py, showast.py, and simparse.py. A prototype 
CLE preprocessor using python Lark and clang is in qd_cle_preprocessor.py.

You can run the pre-processor as follows:

```
sudo -H pip3 install lark-parser
./qd_cle_preprocessor.py -f helloworldmars-cle.cpp

```


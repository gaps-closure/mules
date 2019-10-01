#include <iostream>

using namespace std;

#pragma cle def HIGH_1 {\
  "level":"SECRET", \
  "cdf": [\
    {"remotelevel":">=SECRET", \
     "direction": "egress", \
     {"guardhint": { "oneway": true}}},\
    {"remotelevel":"<SECRET", \
     "direction": "egress", \
     {"guardhint": { "downgrade": "user-defined"}}}\
  ]\
}

string prt() 
{
  #pragma cle HIGH_1
  string str = "Hello World, Mars!";
  return str;
}

int main()
{
  cout << prt() << endl;
  return 0;
} 


#include <iostream>

using namespace std;

#pragma cle def HIGH_1 R"JSON({
  "level":"SECRET", 
  "cdf": [
    {"remotelevel":">=SECRET", 
     "direction": "egress", 
     {"guardhint": { "oneway": true}}},
    {"remotelevel":"<SECRET", 
     "direction": "egress", 
     {"guardhint": { "downgrade": "user-defined"}}}
  ]
}
)JSON"

string prt() 
{
  #pragma cle begin HIGH_1
  string str = "Hello World, Mars!";
  #pragma cle end HIGH_1
  return str;
}

int main()
{
  cout << prt() << endl;
  return 0;
} 


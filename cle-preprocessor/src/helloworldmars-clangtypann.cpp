#include <iostream>

using namespace std;

string prt() 
{
  string __attribute__((type_annotate("HIGH_1"))) str = "Hello World, Mars!";
  return str;
}

int main()
{
  cout << prt() << endl;
  return 0;
}


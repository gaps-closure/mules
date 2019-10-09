#include <iostream>
#define type_annotate(X) annotate(X)

using namespace std;

string x = "Hello World, Mars!\n";

class __attribute__((type_annotate("TAINT_CLASS_1"))) PreciseLocation {
  public:
    double lat;
    double lon;
    double __attribute__((type_annotate("TAINT_FIELD_1"))) alt;
};

typedef struct Location_ {
    int __attribute__((type_annotate("TAINT_FIELD_2"))) locCode;
} __attribute__((type_annotate("TAINT_TYPEDEF_1"))) Location;

// tyann from prt function declaration NOTEMITTED
void __attribute__((type_annotate("TAINT_FUN_1"))) prt(string y)
{
  cout << y;
}

int main()
{
  // tyann EMITTED on loc1 variable declaration
  // tyann from PreciseLocation class declaration NOTEMITTED 
  PreciseLocation * __attribute__((type_annotate("TAINT_VAR_1"))) loc1;

  // tyann from PreciseLocation class declaration NOTEMITTED
  PreciseLocation loc2;

  // tyann from Location typedef NOTEMITTED
  Location ll;

  // tyann from Location_.locCode field declaration EMITTED on field access
  ll.locCode = 42;

  // tyann from PreciseLocation.alt field declaration EMITTED on field access
  loc2.alt = 29.0;

  // tyann from loc1 variable declaration EMITTED on variable access
  // tyann from PreciseLocation class declaration NOTEMITTED 
  loc1 = &loc2;

  // tyann from PreciseLocation.alt field declaration EMITTED on field access
  loc2.alt = 30.0;

  // tyann from loc1 variable declaration EMITTED on variable access
  // tyann from PreciseLocation.alt field declaration EMITTED on field access
  loc1->alt = 28.0;

  // tyann EMITTED on secretKey variable declaration
  char * __attribute__((type_annotate("TAINT_VAR_2"))) secretKey;

  // tyann from secretKey variable declaration EMITTED on variable access
  char * foo = secretKey;

  // tyann from prt function declaration NOTEMITTED
  prt(x);

  // typedef int * __attribute__((type_annotate("TAINT_TYPEDEF_2"))) TaintVar;
  // TaintVar ttt;
  
  return 0;
}


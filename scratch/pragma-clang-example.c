#include <stdio.h>

char *x ="Hello World, Mars!\n";

typedef struct precise_coords {
  double lat;
  double lon;
  double alt;
} *PreciseLocation;

int main()
{
  #pragma clang attribute push (__attribute__((type_annotate("HIGH1"))), apply_to = any(function,type_alias,record,enum,variable,field))
  PreciseLocation loc1;
  PreciseLocation loc2;
  #pragma clang attribute pop

  #pragma clang attribute push (__attribute__((annotate("HIGH2"))), apply_to = any(function,type_alias,record,enum,variable,field))
  char * secretKey;
  #pragma clang attribute pop

  char * foo = secretKey;
  printf("%s",x);
  return 0;
}


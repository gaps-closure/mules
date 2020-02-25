#include <stdio.h>
//#define type_annotate(X) annotate(X)

char *x ="Hello World, Mars!\n";

typedef struct precise_coords {
  double lat;
  double lon;
  double alt;
} * __attribute__((type_annotate("HIGH_1"))) PreciseLocation;

int main()
{
  PreciseLocation loc1;
  PreciseLocation loc2;
  char * __attribute__((type_annotate("HIGH_2"))) secretKey;
  char * foo = secretKey;
  printf("%s",x);
  return 0;
}


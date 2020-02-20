#include <stdio.h>

#define TAINTED __attribute__((type_annotate("tainted")))

typedef struct coarse_coords2 {
  double lat;
} TAINTED CoarseLocation2;

#pragma mark 

// Developer defines a CLE label
// Supporting JSON in \-escaped lines
#pragma cle def HIGH_1 {\
  "level":"SECRET", \
  "cdf": [\
    {"remotelevel":">=SECRET", \
     "direction": "egress", \
     "guardhint": { "oneway": "true"}},\
    {"remotelevel":"<SECRET", \
     "direction": "egress", \
     "guardhint": { "downgrade": "user-defined"}}\
  ] }

int main()
{
  CoarseLocation2 loc4;

  return 0;
}


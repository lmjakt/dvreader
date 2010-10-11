#include "matrix.h"

void increment_matrix(float* a, float* b, unsigned int l)
{
  float* end = a + l;
  while(a < end){
    (*a) += (*b);
    ++a;
    ++b;
  }
}


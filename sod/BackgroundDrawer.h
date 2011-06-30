#ifndef BACKGROUNDDRAWER_H
#define BACKGROUNDDRAWER_H

class PosInfo;
struct dpoint;

#include <vector>


class BackgroundDrawer
{
 public:
  BackgroundDrawer(std::vector<dpoint*> points, PosInfo* pos);
  ~BackgroundDrawer();

  // returns an argb image of the current pos size
  // color_matrix should have one row for each dimension in dims, and 4
  // entries, alpha, red, green, blue.
  unsigned char* simple_gaussian(std::vector<unsigned int> dims, 
				 unsigned char* color_matrix, float var);

 private:
  std::vector<dpoint*> points;
  PosInfo* pos;
  
  float* gaussianf(unsigned int dim, float var);
  float maxValue(float* f, size_t s);
  
};

#endif

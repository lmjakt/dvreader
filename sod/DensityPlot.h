#ifndef DENSITYPLOT_H
#define DENSITYPLOT_H

#include <vector>

class PosInfo;

class DensityPlot {
 public:
  DensityPlot(std::vector<float> x, std::vector<float> y, PosInfo* pos);
  
  //  unsigned short* densityPlot(unsigned int width, unsigned int height, int radius, unsigned short* max_value=0);
  unsigned short* densityPlot(int radius, unsigned short* max_value=0);
  // densityPlot uses a flat circle of radius r. We can also define a g_DensityPlot using
  // 2D kernel, but try this first.

 private:
  std::vector<float> points_x;
  std::vector<float> points_y;
  
  //  float x_min, x_max;
  // float y_min, y_max;

  PosInfo* pos_info;
  
};

#endif

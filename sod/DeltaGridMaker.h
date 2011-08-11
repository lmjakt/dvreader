#ifndef DELTAGRIDMAKER_H
#define DELTAGRIDMAKER_H

#include <vector>

struct dpoint;

// making a delta grid (a grid to show how the gradation of values in nodes) needs to
// independent steps

// 1:
// for each point make a 2 dimensional dpoint that contains one component vector for each
// dimension of the initial phase space. In order to do this, compare each point to every other point 

// 2:
// Make a grid of dpoints, and then assign these some sort of localised average values

// In both steps, one needs to use some sort of kernel function to keep the deltas localised.
// I quite like the idea of a sharpened sigmoid function.. 
// something like
//
// f(d) = 1 - (1 / (1 + exp(-(x-1)*4)) )
// where d, the distance is defined as the inter grid node distance. 
// for values of d (0, 1, 2, 3)  f(d) = (0.98, 0.5, 0.018, 0.0003)
// the multiplier and the subtractor can perhaps be set by the user..
// but default values of 1 and 4 seem to me to give reasonable values..

class DeltaGridMaker {
 public:
  DeltaGridMaker();
  ~DeltaGridMaker();

  std::vector<dpoint*> makeGrid(std::vector<dpoint*>& points, unsigned int grid_dim);

 
  
};

#endif

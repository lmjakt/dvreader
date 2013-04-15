#ifndef OCL_DISTANCEMAPPER_H
#define OCL_DISTANCEMAPPER_H

#include "../open_cl/oCL_base.h"
#include "distanceMapper.h"
#include <vector>

class OCL_DistanceMapper : public OCL_base
{
 public:
  OCL_DistanceMapper();
  ~OCL_DistanceMapper();

  std::vector<stressInfo> reduce_dimensions(float* points, unsigned int node_no, 
					    unsigned int starting_dimensionality,
					    unsigned int target_dimensionality, 
					    unsigned int iterations,
					    float* distances);
 private:
  
  float* dimFactors;
  unsigned int dimensionality;
  unsigned int t_dimensionality;

  void shrink_dimensionality(unsigned int iter_no);

};

#endif

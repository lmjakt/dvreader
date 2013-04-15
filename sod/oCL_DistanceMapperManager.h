#ifndef OCL_DISTANCEMAPPERMANAGER_H
#define OCL_DISTANCEMAPPERMANAGER_H

class node_set;
class OCL_DistanceMapper;

class OCL_DistanceMapperManager {
 public:
  OCL_DistanceMapperManager(node_set* pos, node_set* dist);
  ~OCL_DistanceMapperManager();
  
  // testing function, returns nothing..always starts with the full positions
  // in N-dimensional space. Get the function to print out some timing info
  void reduce_dimensions(unsigned int iter_no, unsigned int target_dim);

 private:
  OCL_DistanceMapper* mapper;

  unsigned int node_no;
  unsigned int dimensionality;
  float* nodes;
  float* node_distances;

};

#endif

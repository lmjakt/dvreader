#include "oCL_DistanceMapperManager.h"
#include "node_set.h"
#include "oCL_DistanceMapper.h"
#include <iostream>
#include <sstream>


OCL_DistanceMapperManager::OCL_DistanceMapperManager(node_set* pos, node_set* dist)
{
  nodes = node_distances = 0;
  node_no = pos->n_size();
  dimensionality = pos->n_dim();
  mapper = 0;
  if(!node_no || !dimensionality){
    std::cerr << "OCL_DistanceMapperManager empty positions " 
	      << pos->n_size() << " * " << pos->n_dim() << std::endl;
    return;
  }
  
  if(dist->n_size() != node_no || dist->n_dim() != (node_no)){
    std::cerr << "OCL_DistanceMapperManager dist has unsuitable dimensions " 
	      << "node_no: " << node_no << "  dist_dim " << dist->n_dim() << std::endl;
    return;
  }
  
  nodes = new float[node_no * dimensionality];
  node_distances = new float[node_no * node_no];

  for(uint i=0; i < node_no; ++i){
    for(uint j=0; j < dimensionality; ++j)
      nodes[ i * dimensionality + j] = pos->value(i, j);
    for(uint j=0; j < node_no; ++j)
      node_distances[i * node_no + j] = dist->value(i, j);
  }
  std::ostringstream define_statement;
  define_statement << "#define DIM_NO " << dimensionality << "\n"
		   << "#define NODE_NO " << node_no;

  mapper = new OCL_DistanceMapper(define_statement.str());
  mapper->device_properties();
  mapper->kernel_properties();
}

OCL_DistanceMapperManager::~OCL_DistanceMapperManager()
{
  delete mapper;
  delete []nodes;
  delete []node_distances;
}


void OCL_DistanceMapperManager::reduce_dimensions(unsigned int iter_no, unsigned int target_dim, 
						  unsigned int local_work_size)
{
  mapper->set_local_item_size(local_work_size);
  mapper->reduce_dimensions(nodes, node_no, dimensionality, target_dim,
			    iter_no, node_distances);
}

void OCL_DistanceMapperManager::print_pointers()
{
  std::cout << "nodes:          " << nodes << std::endl;
  std::cout << "node_distances: " << node_distances << std::endl;
}

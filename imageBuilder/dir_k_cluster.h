#ifndef DIR_K_CLUSTER
#define DIR_K_CLUSTER

// A class that implements a directional k-cluster using weighted
// unit vectors.
//

// This is done in order to attempt to create a model that can allow voxels to
// be directly assigned to gene identities plus some error (or correlation).
// Essentially we're trying to extract the spectral identities from the data.


// The weighting is used to give prominence to longer vectors. This is done
// since most members (voxels) should contain noise only.

// This might mean that we cannot model background noise very effectively.
// But we'll see what turns up.. 

// The big question is as to whether data from individual channels ought to
// be normalised first, and if so, in what manner to do that normalisation.
// z-score seems reasonable, but.. then we'll need to remembe the data
// afterwards

// lets try by z-scoring the data first, and remembering what happens.

#include <vector>

class ImStack;

class Dir_K_Cluster
{
 public:
  Dir_K_Cluster(ImStack* stack);  // converts to a normalised vector format.
  ~Dir_K_Cluster();
  void cluster(unsigned int k, unsigned int iter);

  std::vector<std::vector<float> > clusterCenters();
  std::vector<double> channel_means();
  std::vector<double> channel_variances();
  ImStack* vector_data();

 private:
  void init(ImStack* stack);
  void unitise();  // sets vector to have unit length, and adds a magnitude channel to the v_data.
  void init_centers();
  unsigned long assign_membership();
  void calculate_centers();
  float e_dis(std::vector<float>& c, unsigned long p);
  
  ImStack* v_data;   // vector data. Contains an additional channel (magnitude).
  unsigned long data_length;
  std::vector<unsigned short> cluster_membership;
  std::vector<double> means;
  std::vector<double> variances;

  std::vector<std::vector<float> > centers; // the directions
  std::vector<float> cluster_sizes;  // number of members to a cluster (float as it uses magnitude)

};

#endif

#include "dir_k_cluster.h"
#include "imStack.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>
#include "../dataStructs.h"


Dir_K_Cluster::Dir_K_Cluster(ImStack* stack)
{
  init(stack);
}

Dir_K_Cluster::~Dir_K_Cluster()
{
  delete(v_data);
}

void Dir_K_Cluster::cluster(unsigned int k, unsigned int iter)
{
  centers.resize(k);
  cluster_sizes.resize(k);
  
  init_centers();
  assign_membership();
  for(unsigned int i=0; i < iter; ++i){
    calculate_centers();
    unsigned long changes = assign_membership();
    std::cout << "Clustering " << i << "\t" << changes << std::endl;
    if(!changes){
      std::cout << "Dir_K_Cluster::cluster " << i << " / " << iter << " but no modification to clusters " << std::endl;
      break;
    }
  }
}

std::vector<std::vector<float> > Dir_K_Cluster::clusterCenters()
{
  return(centers);
}

ImStack* Dir_K_Cluster::vector_data()
{
  return(v_data);
}

std::vector<double> Dir_K_Cluster::channel_means()
{
  return(means);
}

std::vector<double> Dir_K_Cluster::channel_variances()
{
  return(variances);
}

void Dir_K_Cluster::init(ImStack* stack)
{
  int x, y, z;
  unsigned int w, h, d;
  stack->pos(x, y, z);
  stack->dims(w, h, d);
  data_length = w * h * d;
  v_data = new ImStack(x, y, z, w, h, d);
  cluster_membership.resize(data_length);
  for(unsigned int i=0; i < stack->ch(); ++i){
    //unsigned long l = w * h * d;
    float* dest = new float[ data_length ];
    float* source = stack->stack(i);
    double sum = 0;
    double sum_of_sq = 0;
    for(unsigned int j=0; j < data_length; ++j){
      dest[j] = source[j];
      sum += dest[j];
      sum_of_sq += (dest[j] * dest[j]);
    }
    double mean = sum / (double)data_length;
    // SS = sum_of_sq - (sum^2/n)
    double SS = sum_of_sq - ((sum * sum) / (double)data_length);
    double std = sqrt(SS / data_length);
    means.push_back(mean);
    variances.push_back(std);
    for(unsigned int j=0; j < data_length; ++j)
      dest[j] = (dest[j] - mean) / std;
    v_data->addChannel(dest, stack->cinfo(i));
  }
  unitise();
}

// Each voxel is considered as a vector composed of the individual channels.
// The unitise() function converts each vector to have a length of one, by
// dividing each individual value by the length of the vector.
// The length of the vector is then stored in an additional channel in the
// the ImStack structure. 
void Dir_K_Cluster::unitise()
{
  unsigned int chno = v_data->ch();
  float* magnitude = new float[data_length];
  // for speed, make local copies of pointers
  std::vector<float*> data;
  for(unsigned int i=0; i < v_data->ch(); ++i)
    data.push_back(v_data->stack(i));
  
  for(unsigned long i=0; i < data_length; ++i){
    float sqsum = 0;
    for(unsigned int j=0; j < chno; ++j)
      sqsum += (data[j][i] * data[j][i]);
    magnitude[i] = sqrt(sqsum);

    //    for(unsigned int j=0; j < chno; ++j)
    //  data[j][i] /= magnitude[i];

  }
  // color will default to white.
  // wave_index should be set to something reasonable, or some sort of
  // const variable or enum. But for now leave it at this.
  v_data->addChannel(magnitude, channel_info(500, fluorInfo(0, 0, 0)));
  
}

void Dir_K_Cluster::init_centers()
{
  srand48(time(0));  // in fact not very random, but good enough.
  // double drand48();
  for(unsigned int i=0; i < centers.size(); ++i){
    centers[i].resize( v_data->ch() - 1 );  // the last channel gives the magnitude
    for(unsigned int j=0; j < centers[i].size(); ++j){
      float sign = mrand48() > 0 ? 1.0 : -1.0;
      centers[i][j] = sign * (5.0 * drand48());       // within 5 standard deviations
    }
  }
}

unsigned long Dir_K_Cluster::assign_membership()
{
  unsigned long change_count = 0;
  for(unsigned long i=0; i < data_length; ++i){
    float min_d = e_dis(centers[0], i);
    unsigned short membership = 0;
    //    std::cout << "initial distance : " << min_d << std::endl;
    for(unsigned int j=1; j < centers.size(); ++j){
      float d = e_dis(centers[j], i);
      //std::cout << "\t" << d << "\t" << membership << std::endl;
      if(d < min_d){
	min_d = d;
	membership = j;
      }
    }
    if(cluster_membership[i] != membership)
      ++change_count;
    cluster_membership[i] = membership;
  }
  return(change_count);
}

void Dir_K_Cluster::calculate_centers()
{
  for(unsigned int i=0; i < centers.size(); ++i){
    cluster_sizes[i] = 0;
    for(unsigned int j=0; j < centers[i].size(); ++j)
      centers[i][j] = 0;
  }
  for(unsigned long i=0; i < data_length; ++i){
    unsigned int c = cluster_membership[i];
    float magnitude = fabs(v_data->lv(centers[c].size(), i));
    for(unsigned int j=0; j < centers[c].size(); ++j){
      centers[c][j] += ( v_data->lv(j, i) * magnitude );
      cluster_sizes[c] += magnitude;
    }
  }
  for(unsigned int i=0; i < centers.size(); ++i){
    for(unsigned int j=0; j < centers[i].size(); ++j){
      if(cluster_sizes[i])
	centers[i][j] /= cluster_sizes[i];
    }
  }
}

float Dir_K_Cluster::e_dis(std::vector<float>& c, unsigned long p)
{
  float d = 0;
  for(unsigned i=0; i < c.size(); ++i){
    float v = v_data->lv(i, p);
    d += (v - c[i])*(v - c[i]);
  }
  return(sqrt(d));
}

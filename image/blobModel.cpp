#include "blobModel.h"
#include <math.h>
#include <string.h>

using namespace std;

BlobModel::BlobModel(int x, int y, int z, float pv, int xy_radius, int z_radius)
{
  xyRadius = fabs((float)xy_radius);
  zRadius = fabs((float)z_radius);  // makes it > 0 !! 
  peakValue = pv;
  xp = (float)x;
  yp = (float)y;
  zp = (float)z;
}

// Use the below to incorporate data from many blobs in the same
// model.
void BlobModel::setPeak(int x, int y, int z, float pv)
{
  peakValue = pv;
  xp = (float)x;
  yp = (float)y;
  zp = (float)z;
}

bool BlobModel::addPoint(int x, int y, int z, float v)
{
  float s_dist = dist(x, y);
  float z_dist = fabsf((float)z - zp);
  if(s_dist > xyRadius || z_dist > zRadius)
    return(false);
  points.push_back(bmPoint(s_dist, (float)(z - zp), v/peakValue));
  return(true);
}

// returns a float* of length (1 + s_range) * (1 + z_radius * 2)
// The dimensions are due to the fact that the numbers will vary from
// 0 -> s_range
// -z_diam -> +z_diam
// including the final value.

// note that there is no guarantee that there is equal coverage for the 
// different positions in the model. Still, maybe we can ignore that for now.

float* BlobModel::model(int& s_range, int& z_radius)
{
  s_range = (int)xyRadius;
  z_radius = (int)zRadius;
  int z_diam = 1 + (int)z_radius * 2;
  float* model = new float[ (1 + s_range) * z_diam ];
  memset((void*)model, 0, sizeof(float) * (1 + s_range) * z_diam);
  for(unsigned int i=0; i < points.size(); ++i){
    int offset = (points[i].dz + (int)zRadius) * (1 + s_range) + points[i].dxy;
    model[offset] += (points[i].pp / (float)points.size());   // not really sure whether I should divide by points.size()
  }
  return(model);
}

float BlobModel::dist(int x, int y)
{
  return( sqrt( ((float)x - xp)*((float)x - xp) + ((float)y - yp)*((float)y - yp) ) );
}

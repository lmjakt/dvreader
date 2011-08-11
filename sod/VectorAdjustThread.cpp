#include "VectorAdjustThread.h"
#include "distanceMapper.h"

VectorAdjustThread::VectorAdjustThread(std::vector<dpoint*>& points, 
				       std::vector<std::vector<float> >& distances,
				       float* dimFactors, bool linear,
				       unsigned int beg, unsigned int end, bool useSlowMethod)
  : points(points), distances(distances), dimFactors(dimFactors), linear(linear), beg(beg), end(end), useSlowMethod(useSlowMethod)
{
  p_size = points.size();
  end = end > p_size ? p_size : end;
  end = end < beg ? beg : end;
  stress = 0;
  subset_length = 0;
}

VectorAdjustThread::~VectorAdjustThread()
{
}

void VectorAdjustThread::setCompareSubset(std::vector<unsigned int*>& comp_sub, unsigned int subLength)
{
  if(!points.size())
    return;
  if(comp_sub.size() == points.size() && subLength < points.size() && subLength > 0){
    compare_subset = comp_sub;
    subset_length = subLength;
  }
}

float VectorAdjustThread::totalStress()
{
  return(stress);
}

void VectorAdjustThread::run()
{
  if(useSlowMethod){
    if(subset_length && compare_subset.size() == points.size()){
      runSubsetSlow();
    }else{
      runSlow();
    }
    return;
  }
  if(subset_length && compare_subset.size() == points.size()){
    runSubsetFast();
  }else{
    runFast();
  }
}

void VectorAdjustThread::runSlow()
{
  float s=0;
  for(unsigned int i=beg; i < end; ++i){
    for(unsigned int j=0; j < p_size; ++j){
      if(i != j)
	s = points[i]->adjustVectors(points[j], distances[i][j], dimFactors, linear, j);
    }
    stress += s;
  }
}

void VectorAdjustThread::runSubsetSlow()
{
  float s=0;
  unsigned int ci=0;
  for(unsigned int i=beg; i < end; ++i){
    for(unsigned int j=0; j < subset_length; ++j){
      ci = compare_subset[i][j];
      s = points[i]->adjustVectors(points[ ci ], distances[i][ci], dimFactors, linear, j);
    }
    stress += s;
  }
}

void VectorAdjustThread::runFast()
{
  float s=0;
  float* buffer = new float[ points[beg]->dimNo ];
  for(unsigned int i=beg; i < end; ++i){
    for(unsigned int j=0; j < p_size; ++j){
      if(i != j)
	s = points[i]->adjustVectorsFast(points[j], distances[i][j], dimFactors, buffer);
    }
    stress += s;
  }
}

void VectorAdjustThread::runSubsetFast()
{
  float s=0;
  float* buffer = new float[ points[beg]->dimNo ];
  unsigned int ci=0;
  for(unsigned int i=beg; i < end; ++i){
    for(unsigned int j=0; j < subset_length; ++j){
      ci = compare_subset[i][j];
      s = points[i]->adjustVectorsFast(points[ci], distances[i][ci], dimFactors, buffer);
    }
    stress += s;
  }
}

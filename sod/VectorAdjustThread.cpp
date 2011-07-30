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
}

VectorAdjustThread::~VectorAdjustThread()
{
}

float VectorAdjustThread::totalStress()
{
  return(stress);
}

void VectorAdjustThread::run()
{
  if(useSlowMethod){
    runSlow();
    return;
  }
  runFast();
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

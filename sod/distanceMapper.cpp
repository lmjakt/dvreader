//Copyright Notice
/*
    eXintegrator integrated expression analysis system
    Copyright (C) 2004  Martin Jakt & Okada Mitsuhiro
  
    This file is part of the eXintegrator integrated expression analysis system. 
    eXintegrator is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version. 

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#include "distanceMapper.h"
#include "VectorAdjustThread.h"
#include <qthread.h>
#include <math.h>
#ifdef Q_OS_MACX
#include <limits.h>
#include <float.h>
#define MINFLOAT (FLT_MIN)
#define MAXFLOAT (FLT_MAX)
#else
#include <values.h>
#endif
#include <qmutex.h>
#include <qapplication.h>
#include <vector>
#include <map>
#include <set>
#include <iostream>
#include <stdlib.h>
#include <time.h>



using namespace std;

dpoint::dpoint(){
  index = 0;
  stress = 0;
  coordinates = new float[1];
  forceVectors = new float[1];
  dimNo = 1;
  components = new componentVector*[1];
  componentNo = 0;
  componentSize = 1;
  //  forceMagnitudeSum = 0;
}

dpoint::dpoint(int i, unsigned int dimensions, unsigned int compNo){
  index = i;
  stress = 0;
  dimNo = dimensions;
  coordinates = new float[dimNo];
  forceVectors = new float[dimNo];
  components = 0;
  if(compNo)
    components = new componentVector*[compNo];     // where do I add these, I'm not sure.. we'll have to work it out later.. 
  //memset((void*)components, 0, sizeof(componentVector*) * compNo); 
  componentSize = compNo;
  componentNo = componentSize;
  for(unsigned int i=0; i < componentSize; ++i){
    components[i] = new componentVector(dimNo);
  }
  //  coordinates.resize(dimensions);
  // forceVectors.resize(dimensions);
  for(unsigned int i=0; i < dimNo; i++){
    coordinates[i] = 0;
    forceVectors[i] = 0;
  }
  //  forceMagnitudeSum = 0;
}

dpoint::~dpoint(){
  for(unsigned int i=0; i < componentNo; i++){
    delete components[i];
  }
  delete []components;  // should just delete the pointers, not the thing they point to (which are other pointers.. )
  delete []forceVectors;
  delete []coordinates;
}

dpoint* dpoint::copy(bool includeComponents){
  // make a new thingy and copy it.. 
  dpoint* np = new dpoint(index, dimNo, componentSize);        
  np->stress = stress;
  np->position = position;
  memcpy(np->coordinates, coordinates, sizeof(float)*dimNo);
  memcpy(np->forceVectors, forceVectors, sizeof(float)*dimNo);
  np->neighbor_indices = neighbor_indices;
  //  np->forceMagnitudeSum = forceMagnitudeSum;
  if(!includeComponents)
    return(np);
  for(unsigned int i=0; i < componentNo; ++i)
    np->components[i] = components[i]->copy();
  return(np);
}

void dpoint::assignValues(dpoint* p){
  if(dimNo > p->dimNo || componentSize > p->componentSize){
    cerr << "dpoint::assignValues unable to assign values since point sizes do not appear to match" 
	 << endl;
    cerr << "this : " << dimNo << "  " << componentSize << "  p: " 
	 << (long)p << "  "  << p->dimNo << "  " << p->componentSize << endl;
    exit(1);
  }
  p->position = position;
  /// possible memory leak here. But deleting the pointers would be a possible seg fault. so.. 
  p->componentNo = componentNo;
  memcpy(p->coordinates, coordinates, sizeof(float) * dimNo);
  memcpy(p->forceVectors, forceVectors, sizeof(float) * dimNo);
  p->neighbor_indices = neighbor_indices;
  //  p->forceMagnitudeSum = forceMagnitudeSum;
  p->stress = stress;
  for(unsigned int i=0; i < componentNo; ++i){
    p->components[i]->attractive = components[i]->attractive;
    p->components[i]->forceNo = components[i]->forceNo;   /// But this seems very inadvisable.
    memcpy(p->components[i]->forces, components[i]->forces, sizeof(float) * components[i]->forceNo);
  }
}


// Since we don't need to add any more memory, we can just decrease the number that tell us how
// many dimensions we are using. For the dpoint that is just changing the dimNo itself. For the
// componentVectors, all that is necessary is to change forceNo.
// note that I don't use this function anymore. Instead I use dimFactors
int dpoint::shrinkDimNo(uint s){
    if((uint)dimNo - s < 2){
	cerr << "Attempt to shrink dpoint dimNo to " << (uint)dimNo - s << "  foiled. " << endl;
	return(0);
    }
    dimNo = (uint)dimNo - s;
    for(unsigned int i = 0; i < componentNo; ++i){
	components[i]->forceNo = dimNo;
    }
    return(dimNo);
}

void dpoint::addComponent(componentVector* cv){
  //void dpoint::addComponent(float* f, int n, bool a){
  //  cout << "point with address : " << (long)this << "  adding component vector component No " << componentNo << "  size : " << componentSize << endl;
    if(componentNo < componentSize){
      if(components[componentNo]){
	delete components[componentNo];  // This should be safe in C++ as long as the pointer is set to 0.
	cerr << "dpoint::addComponent deleting previous component. Unsuspected behaviour" << endl;
      }
      components[componentNo] = cv;
      componentNo++;
      cout << "Added a new component size is now : " << componentNo << endl;
      return;
    }
    cout << endl << "\t\t\tGROWING THE COMPONENTS FROM " << componentSize << "  to " << componentSize * 2 + 1 << endl << endl;
    componentSize = componentSize * 2 + 1;    // just in case we start with 0.
    componentVector** newComps = new componentVector*[componentSize];
    memcpy(newComps, components, sizeof(componentVector*) * componentNo);  // which should be ok..
    newComps[componentNo] = cv;
    componentNo++;
    delete []components;
    components = newComps;
}

// Note that dimFactors has to have the same size as the number of dimensions
// or to be completely correct has to be as large as dimNo or we'll end up
// getting out of bounds values
float dpoint::adjustVectors(dpoint* p, float d, float* dimFactors, bool linear, unsigned int comp_index){  // include unsigned int index..
  if(dimNo != p->dimNo){
    cerr << "point coordinate mismatch size thingy : " << endl;
    return(stress);
  }
  // this function requires that component forces have already been set up. Failure to do so will lead to
  if(comp_index >= componentNo){
    cerr << "dpoint::adjustVectors no components created returning without adjustment" << endl;
    return(0);
  }
  float* coordDists = components[comp_index]->forces;  // this must be ok. or we should crash.. 
  float D = 0;                 // the actual euclidean distance.. 
  for(unsigned int i=0; i < dimNo; i++){
    coordDists[i] = dimFactors[i] * (p->coordinates[i] - coordinates[i]);
    // coordDists[i] = (!coordDists[i]) ? MINFLOAT : coordDists[i]; // stabilises but slows down.
    D += (coordDists[i] * coordDists[i]);
  }
  D = sqrt(D);

  float delta = (D- d); // linear  by the fold difference.. 
  D = (D <= 0) ? MINFLOAT : D; 
  // adjust the force vectors by some measure
  stress += fabs(delta);     // the absolute amount of stress on the point.. 
  //float f_mag = 0;
  //float f_mag_c = 0;
  float* compForces = coordDists;
  for(unsigned int i=0; i < dimNo; ++i){
    //f_mag_c = (delta * coordDists[i])/D;
    forceVectors[i] += (delta * coordDists[i])/D;
    compForces[i] = (delta * coordDists[i])/D;   // strictly speaking we don't need this one.. 
    components[comp_index]->attractive = (delta > 0);
    //f_mag += (f_mag_c * f_mag_c);
  }
  
  //forceMagnitudeSum += sqrt(f_mag);

  return(stress);     // not really useful as it's an intermediate value.. 
}

float dpoint::adjustVectorsFast(dpoint* p, float d, float* dimFactors, float* buffer)
{
  if(dimNo != p->dimNo){
    cerr << "point coordinate mismatch size thingy : " << endl;
    return(stress);
  }
  // use the provided buffer for the coordDists (in order not to new and delete)
  float* coordDists = buffer;

  float D = 0;                 // the display space euclidean distance.. 
  for(unsigned int i=0; i < dimNo; ++i){
    coordDists[i] = dimFactors[i] * (p->coordinates[i] - coordinates[i]);
    D += (coordDists[i] * coordDists[i]);
  }
  D = sqrt(D);   // alleluliahh

  float delta = (D-d);
  //  forceMagnitudeSum += delta;
  D = (D <= 0) ? MINFLOAT : D; 
                                  // adjust the force vectors by some measure
  stress += fabs(delta);     // the absolute amount of stress on the point.. 

  for(unsigned int i=0; i < dimNo; ++i){
    forceVectors[i] += (delta * coordDists[i])/D;
  }
  return(stress);     // not really useful as it's an intermediate value.. 
}

float dpoint::move(float forceMultiplier){
  float distanceMoved = 0;
  ////// Experimental
  /////// instead of using a static forceMultipler use the term:
  //////  0.5 * forceMagnitude / forceMagnitudeSum
  // if all vectors point the same direction, then forceMagnitude = forceMagnitudeSum
  // the only issue is that we have to calculate the forceMagnitude here

  // this idea doesn't work the forcemultiplier should simply be 1 / (n * 2)
  // where n is the number of comparisons per point. (well 1 / n if not recipocral).
  //  float forceMagnitude = 0;
  //for(unsigned int i=0; i < dimNo; ++i)
  //  forceMagnitude += (forceVectors[i] * forceVectors[i]);
  //forceMagnitude = sqrt(forceMagnitude);
  // and override forceMultiplier..
  //forceMultiplier = 0.5 * forceMagnitude / forceMagnitudeSum;
  //  cout << "forceMultipler : " << forceMultiplier << " = " << 0.5 << " * " << forceMagnitude << " / " << forceMagnitudeSum << endl;
  for(unsigned int i=0; i < dimNo; i++){
    coordinates[i] += forceMultiplier * forceVectors[i];
    distanceMoved += ((forceMultiplier * forceVectors[i]) * (forceMultiplier * forceVectors[i]));
  }
  return(sqrt(distanceMoved));     // i.e. the euclidean distance.. 
}


void dpoint::resetForces(){              /// THIS GIVES RISE TO A MEMORY LEAK IF NOT USED WITH EXTREME CARE AS IT LEAVES LOTS OF HANGING POINTERS
  stress = 0;
  for(unsigned int i=0; i < dimNo; i++){
    forceVectors[i] = 0;
  }
  //  forceMagnitudeSum = 0;
  //  componentNo = 0;        // componentSize still holds what has been assigned.
}

void dpoint::setPos(vector<float> coords){
  position = coords;
}

DistanceMapper::DistanceMapper(vector<int> expI, vector<vector<float> > d, QMutex* mutx, 
			       vector<vector<dpoint*> >* prntPoints, QObject* prnt, 
			       vector<stressInfo>* stressLevels, DimReductionType drt, bool rememberComponents){
  //cout << "beginning of distance mapper constructor " << endl;
  calculating = false;
  expts = expI;
  distances = d;
  parent = prnt;
  parentPoints = prntPoints;
  pointMutex = mutx;
  errors = stressLevels;
  rememberComponentVectors = rememberComponents;

  iterationNo = 0;  // this means nothing happens unless setDim is called at some point.
  dimensionality = 4;  // actually set by setDim, without which we do nothing
  thread_no = 4;        // most CPUs these days seem to have 4 cores.. 
  update_interval = 11;
  currentDimNo = dimensionality;

  // By default we compare everything to everything..
  subset_length = 0;
  use_subset = false;

  // set the dimFactors to 1.
  // Note this will crash if dimensionality is less than 0. 
  dimFactors = 0;
  dimReductionType = drt;
  drtMap.insert(make_pair((int)STARBURST, STARBURST));
  drtMap.insert(make_pair((int)GRADUAL_SERIAL, GRADUAL_SERIAL));
  drtMap.insert(make_pair((int)GRADUAL_PARALLEL, GRADUAL_PARALLEL));
  resetDimFactors();
  
  srand(time(0));
  moveFactor = 1.0 / (float)(distances.size() * 2);      // just keep it below 1,, above one will lead to strange behaviour.. 
  // make sure that the distances are all the same.. 
  if(expts.size() != distances.size()){
    cerr << "bugger everytyhing is going to die man.. " << endl;
    cerr << "distances size is : " << distances.size() << endl
	 << "experiments size is : " << expts.size() << endl;
    exit(1);
  }
  // count the total distance in the thingy.. as counted by the program..
  totalDistance = 0;
  for(uint i=0; i < distances.size(); i++){
    for(uint j=0; j < distances[i].size(); j++){
      totalDistance += distances[i][j];
    }
  }
  reInitialise();  // this creates the points and calls initialise()
}

DistanceMapper::~DistanceMapper(){
  cout << "destryoing the DistanceMapper object, hoho yeahh " << endl;
  delete dimFactors;
  for(unsigned int i=0; i < compare_subsets.size(); ++i)
    delete []compare_subsets[i];
  // I think that I actually need to delete the points vector (it used to
  // be done by the distance viewer, but I don't think that happens at the moment)
}

void DistanceMapper::restart(){
  wait();   // wait for the thread to finish, this could end up taking a long time..
  calculating = true;
  initialisePoints();  // ok lalal 
  //run();    // and start ..
}


//////////// This function is called from the distanceViewer 
//////////// when a point has been moved manually

void DistanceMapper::updatePosition(int i, float x, float y){
  wait();
  //  calculating = true;  -- Not sure about this change.. 
  if(points.size() > (uint)i){
    if(points[i]->dimNo > 1){
      points[i]->coordinates[0] = x;
      points[i]->coordinates[1] = y;
    }
  }
  adjustVectors();
  updateParentPoints();
  movePoints(points);
  resetPoints(points);
}


float DistanceMapper::adjustVectors(bool linear){
  // Looks like we could optimise this one,, in some way.. 
  float totalStress = 0;

  // rewritten to make use of VectorAdjustThread objects.
  unsigned int t_no = thread_no;       // in case it gets changed half-way through..
  t_no = t_no > points.size() ? points.size() : t_no;
  unsigned int group_size = points.size() / t_no;
  vector<VectorAdjustThread*> vecAdjusters(t_no);
  
  // set moveFactor appropriately
  moveFactor = 1.0 / (float)(2 * points.size());
  if(use_subset && compare_subsets.size() == points.size())
    moveFactor = 1.0 / (float)(subset_length);

  for(unsigned int i=0; i < vecAdjusters.size(); ++i){
    unsigned int beg = i * group_size;
    unsigned int end = i < (t_no - 1) ? beg + group_size : points.size();
    vecAdjusters[i] = new VectorAdjustThread(points, distances, dimFactors, linear, beg, end, rememberComponentVectors);
    if(use_subset && compare_subsets.size() == points.size())
      vecAdjusters[i]->setCompareSubset(compare_subsets, subset_length);
  }
  for(unsigned int i=0; i < vecAdjusters.size(); ++i)
    vecAdjusters[i]->start();
  for(unsigned int i=0; i < vecAdjusters.size(); ++i){
    vecAdjusters[i]->wait();
    totalStress += vecAdjusters[i]->totalStress();
    delete vecAdjusters[i];
  }
  return(totalStress);
}

float DistanceMapper::adjustGridVectors(bool linear){
  cerr << "WARNING adjustGridVectors needs to be rewritten to make use of the new adjustVectorsFasta and the VectorAdjustThread class" << endl;
  return(0);
  float stress = 0;
  for(unsigned int i=0; i < gridPoints.size(); ++i){
    for(unsigned int j=0; j < points.size(); ++j)
      stress += gridPoints[i]->adjustVectors(points[j], gridDistances[i][j], dimFactors, linear, j);
  }
  return(stress);
}

float DistanceMapper::movePoints(vector<dpoint*>& pnts){
  float movedDistance = 0;
  for(uint i=0; i < pnts.size(); i++){
    movedDistance += pnts[i]->move(moveFactor);
  }
  return(movedDistance);
}

void DistanceMapper::resetPoints(vector<dpoint*>& pnts){
  for(uint i=0; i < pnts.size(); i++){
    pnts[i]->resetForces();
  }
}

void DistanceMapper::run(){
  calculating = true;
  bool linear = true;
  
  pointMutex->lock();
  errors->resize(iterationNo);
  pointMutex->unlock();
  
  float minError = MAXFLOAT;
  bool errorReduced = false;
  for(int i=0; i < iterationNo; ++i){
    float stress = adjustVectors(linear);
    errorReduced = false;
    if(effectiveDimensionality() == 2.0 && minError > stress){
      minError = stress;
      errorReduced = true;
    }
    pointMutex->lock();
    (*errors)[i].setStress(dimensionality, dimFactors, currentDimNo, stress);
    pointMutex->unlock();
    // update the parent..
    if(!(i % update_interval) || errorReduced)
      updateParentPoints();
    movePoints(points);
    resetPoints(points);
    
    // if we have gridPoints
    if(gridPoints.size()){
      adjustGridVectors(linear);
      movePoints(gridPoints);
      resetPoints(gridPoints);
    }
    
    // Squeeze or eliminate dimensions..
    reduceDimensionality(dimReductionType, i);
  }
  cout << "distanceMapper is done .. minError was " << minError << endl;
  calculating = false; 
}

void DistanceMapper::reInitialise(){
  vector<set<unsigned int> > nbor_sets(points.size());
  for(uint i=0; i < points.size(); ++i){
    nbor_sets[i] = points[i]->neighbor_indices;
    delete points[i];
  }
  points.resize(0);
  points.reserve(expts.size());
  unsigned int comp_vectors = rememberComponentVectors ? expts.size() : 0;
  for(uint i=0; i < expts.size(); i++)
    points.push_back(new dpoint(expts[i], dimensionality, comp_vectors));    

  if(nbor_sets.size() == points.size()){
    for(uint i=0; i < points.size(); ++i)
      points[i]->neighbor_indices = nbor_sets[i];
  }
  initialisePoints();
  resetDimFactors();
  currentDimNo = dimensionality;
  cout << "DistanceMapper::reinitialised with dimNo : " << dimensionality << " and with point # " << points.size() << endl;
}

void DistanceMapper::setDim(int dim, int iter, int drt){
    if(dim < 2 || iter < 2){
	cerr << "DistanceMapper, attempt to set dim to " << dim << "  and iterNo to " << iter << "  failed" << endl;
	return;
    }
    if(drtMap.count(drt)){
	dimReductionType = drtMap[drt];
    }else{
	cerr << "DistanceMapper::setDim unknown drt : " << drt << endl;
    }
    dimensionality = dim;
    iterationNo = iter;
//    resetDimFactors();
    reInitialise();
}

void DistanceMapper::setIter(int iter)
{
  if(iter > 0)
    iterationNo = iter;
}

void DistanceMapper::setThreadNumber(unsigned int tno)
{
  if(!tno || isRunning())
    return;
  
  thread_no = tno;
}

//// Doesn't work at the moment: moveFactor is set dynamically.. 
void DistanceMapper::setMoveFactor(float mf)
{
  moveFactor = mf;
}

void DistanceMapper::setSubset(unsigned int subset_size)
{
  for(unsigned int i=0; i < compare_subsets.size(); ++i)
    delete []compare_subsets[i];
  compare_subsets.resize(0);        // though this is actually a bit silly.
  
  subset_length = subset_size < points.size() ? subset_size : 0;
  if(!subset_length){
    use_subset = false;
    return;
  }
  compare_subsets.resize(distances.size()); // which should always == points.size()
  // the lazy way to sort stuff, use a multi_map..
  // seems like it might be better to select a random subset..
  cout << "RAND_MAX = " << RAND_MAX << "  : " << (subset_length * RAND_MAX) / RAND_MAX << endl; 
  for(unsigned int i=0; i < distances.size(); ++i){
    //multimap<float, unsigned int> mmap;
    set<unsigned int> selection;
      //    for(unsigned int j=0; j < distances[i].size(); ++j)
      //mmap.insert(make_pair(distances[i][j], j));
      //unsigned int count = 0;
    unsigned int* sub_index = new unsigned int[subset_length];
    //cout << i << ":";
    while(selection.size() < subset_length){
      unsigned long j = rand() % points.size();
      if(!selection.count(j)){
	sub_index[ selection.size() ] = j;
	selection.insert(j);
	//	cout << "\t" << j;
      }
    }
    //    cout << endl;
    // for(multimap<float, unsigned int>::iterator it=mmap.begin(); it != mmap.end(); ++it){
    //   if(it->second != i){
    // 	sub_index[count] = it->second;
    // 	++count;
    //   }
    //   if(count >= subset_length)
    // 	break;
    // }
    compare_subsets[i] = sub_index;
  }
  use_subset = true;
}

void DistanceMapper::useSubSet(bool useSub)
{
  use_subset = useSub;
}

void DistanceMapper::setInitialPoints(vector<vector<float> > i_points, unsigned int grid_points){
  if(i_points.size() != points.size()){
    cerr << "DistanceMapper setInitialPoints, i_points size is not good: "
	 << i_points.size() << " != " << points.size();
  }
  // apart from that we can allow anything.. 
  initialPoints = i_points;
  initialisePoints();
  if(grid_points)
    initialiseGrid(grid_points, i_points);
}

void DistanceMapper::setUpdateInterval(unsigned int ui)
{
  update_interval = ui;
}

void DistanceMapper::makeTriangles()
{
  cout << "make Triangles " << endl;
  for(unsigned int i=0; i < points.size(); ++i)
    points[i]->neighbor_indices.clear();
  pair<float, unsigned int> blank_pair = make_pair(MAXFLOAT, 0);
  for(unsigned int i=0; i < points.size(); ++i){
    list< pair<float, unsigned int> > nbors(2, blank_pair);
    for(unsigned int j=0; j < points.size(); ++j){
      if(distances[i][j] > nbors.back().first || i == j)
	continue;

      list< pair<float, unsigned int> >::iterator it = nbors.begin();
      if(distances[i][j] > nbors.front().first)
	++it;
      nbors.insert( it, make_pair(distances[i][j], j) );
      nbors.pop_back();
    }
    if(nbors.size() != 2){
      cerr << "Make Triangles nbors size is not 2, not sure what's going on here" << endl;
      exit(1);
    }
    points[i]->neighbor_indices.insert( nbors.front().second );
    points[i]->neighbor_indices.insert( nbors.back().second );
    points[ nbors.front().second ]->neighbor_indices.insert( nbors.back().second );
    cout << " linked : " << i << " -> " << nbors.front().second << "," << nbors.back().second << endl;
  }
  updateParentPoints();
}

// makes no copy. will delete or modify during mapping.
// use carefully.
std::vector<dpoint*> DistanceMapper::grid()
{
  return(gridPoints);
}

void DistanceMapper::initialisePoints(){
  calculating = true;
  for(uint i=0; i < points.size(); i++){
    if(initialPoints.size() == points.size())
      points[i]->setPos(initialPoints[i]);
    for(uint j=0; j < points[i]->dimNo; j++){
      if(initialPoints.size() == points.size() && initialPoints[i].size() > (uint)j){
	points[i]->coordinates[j] = initialPoints[i][j];
      }else{
	points[i]->coordinates[j] = float(100) * rand()/RAND_MAX;
      }
    }
  }
} 

// 
void DistanceMapper::initialiseGrid(unsigned int pno, vector<vector<float> >& i_points)
{
  unsigned int i_dim = 0;
  // determine the correct number of dimensions, then find ranges for
  // those dimensions.
  for(unsigned int i=0; i < i_points.size(); ++i)
    i_dim = i_points[i].size() > i_dim ? i_points[i].size() : i_dim;
  if(!i_dim){
    cerr << "DistanceMapper::initialiseGrid no dimensions obtained" << endl;
    return;
  }
  vector<float> max_v(i_dim);
  vector<float> min_v(i_dim);
  // set inital mins and maxes
  for(unsigned int i=0; i < i_points.size(); ++i){
    if(i_points[i].size() == i_dim){
      for(unsigned int j=0; j < i_points[i].size(); ++j){
	max_v[j] = i_points[i][j];
	min_v[j] = i_points[i][j];
      }
      break;
    }
  }
  for(unsigned int i=0; i < i_points.size(); ++i){
    for(unsigned int j=0; j < i_points[i].size(); ++j){
      max_v[j] = max_v[j] < i_points[i][j] ? i_points[i][j] : max_v[j];
      min_v[j] = min_v[j] > i_points[i][j] ? i_points[i][j] : min_v[j];
    }
  }
  // then set up points in space. I'm not exactly sure of the best way of doing
  // this, but let's put it into another function that takes the number of points
  // and the max_v and min_v vectors... 
  createGridPoints(pno, min_v, max_v);
}

void DistanceMapper::createGridPoints(unsigned int pno, vector<float> min_v, vector<float> max_v)
{
  if(min_v.size() != max_v.size() || !pno || !min_v.size())
    return;
  // first calculate the total number of grid components.
  unsigned grid_size = pno;
  for(unsigned int i=1; i < min_v.size(); ++i)
    grid_size *= pno;

  vector<float> ranges;
  for(unsigned int i=0; i < min_v.size(); ++i)
    ranges.push_back( max_v[i] - min_v[i] );

  // the position at a given point.
  vector<unsigned int> g_pos(min_v.size(), 0);
  //  vector<unsigned int> g_pos_counters(min_v.size(), 0);
  gridPoints.resize(grid_size);
  gridDistances.resize(grid_size);

  for(unsigned int i=0; i < gridPoints.size(); ++i){
    int d_size = grid_size / pno;
    int remainder = i;
    for(unsigned j = g_pos.size()-1; j < g_pos.size(); --j){  // if j is unsiged should overflow
      g_pos[j] = remainder / d_size;
      remainder -= g_pos[j] * d_size;
      d_size /= pno;
    }
    cout << "gridPoint : " << i;
    for(unsigned int j=0; j < g_pos.size(); ++j)
      cout << "\t" << g_pos[j];
    cout << endl;
    gridPoints[i] = createGridPoint(pno, g_pos, min_v, max_v);
  }
  // Then we need to set up the distances between points and grid_points
  for(unsigned int i=0; i < gridPoints.size(); ++i){
    gridDistances[i].resize(points.size());
    for(unsigned int j=0; j < points.size(); ++j){
      float d = 0;
      for(unsigned int k=0; k < (uint)points[j]->dimNo; ++k){
	d += 
	  ((gridPoints[i]->coordinates[k] - points[j]->coordinates[k]) *
	   (gridPoints[i]->coordinates[k] - points[j]->coordinates[k]));
      }
      gridDistances[i][j] = sqrt(d);
    }
  }

  // setting up the neighbours
  for(unsigned int i=0; i < gridPoints.size(); ++i){
    vector<dpoint*> neighbors(min_v.size(), 0);
    unsigned int inc = 1;
    for(unsigned int j=0; j < neighbors.size(); ++j){
      if(gridPoints[i]->position[j] + ranges[j] <= ((ranges[j] / 100.0) + max_v[j]) &&  i + inc < gridPoints.size())
	neighbors[j] = gridPoints[i + inc];
      inc *= pno;
    }
    gridPoints[i]->neighbors = neighbors;
  }

  // and that should be everything thats necessary. Though I don't know
  // how stable this is to all kinds of input. esp. if dimNo isn't matched
  // with initial points. Anyway, let's see. 
 
}

// g_pos should contain the grid point index position for each dimension
// i.e 0 --> (p_no - 1)    and these should be used to set the position
// in conjunction with the mins & maxes of each dimension
dpoint* DistanceMapper::createGridPoint(unsigned int p_no, vector<unsigned int>& g_pos,
					vector<float>& min_v, vector<float>& max_v)
{
  if(g_pos.size() != min_v.size())
    exit(1);
  vector<float> pos(min_v.size());
  for(unsigned int i=0; i < pos.size(); ++i){
    if(!(max_v[i] - min_v[i])){
      pos[i] = min_v[i];
      continue;
    }
    pos[i] = min_v[i] + float(g_pos[i])/float(p_no-1) * (max_v[i] - min_v[i]);
  }
  dpoint* point = new dpoint(0, dimensionality, 0);   // 0 because these should not remember the components
  point->setPos( pos );
  for(unsigned int i=0; i < (uint)point->dimNo; ++i){
    if(i < pos.size()){
      point->coordinates[i] = pos[i];
    }else{
      point->coordinates[i] = float(100) * rand()/RAND_MAX; // but this should never happen
    }
  }
  return(point);
}

void DistanceMapper::updateParentPoints(){
    // this basically sets the last parent points to something else..
    pointMutex->lock();
    if(!parentPoints->size()){
	parentPoints->push_back(points);
	clonePoints();
	pointMutex->unlock();
	return;
    }
    // Otherwise we go throught the points and assign the appropriate values
    //  cout << "Assigning to the end of parentPoints of size " << parentPoints->size() << endl;
    for(uint i=0; i < parentPoints->back().size() && i < points.size(); ++i){
	points[i]->assignValues(parentPoints->back()[i]);
    }
    pointMutex->unlock();
}
	


// This function should only be used if the points pointers are previously
// copied to somewhere else. As it will not have any way of storing the new pointers
// a better way would be to return a new vector of points.. oh well.. 
void DistanceMapper::clonePoints(){
  // because we are handing over control of the points to the owner, we need to have some new points to 
  // manipulate. Because we are using pointers for everything, we will need to copy the thingy.. probably the best way 
  // would be to provide a copy constructor.. -- using = ,, but hmm,, I think a copy command might be better.. 
//    cout << "clone points .. " << endl;
  for(uint i=0; i < points.size(); i++){
    points[i] = points[i]->copy();    
    // this doesn't copy the componentVectors, nor the stress, 
    // but should be enough.. Note this is could easily lead to memory leaks..
    // but this should only be called once..  
  }
}

// drt defines how we reduce the dimensionality
// it_no is the current iteration and we use that to define whether
// we reduce the dimensionality or not
void DistanceMapper::reduceDimensionality(DimReductionType drt, int it_no){
    float reductionFactor = 1.2;
    
    if(drt == GRADUAL_PARALLEL){
	for(int i=2; i < dimensionality; ++i){
	    dimFactors[i] -= reductionFactor/(float)iterationNo;
	    dimFactors[i] = dimFactors[i] > 0 ? dimFactors[i] : 0;
	}
	return;
    }
    // Else we need to reduce the dimNo...
    currentDimNo = (int)ceil( (float)(iterationNo - it_no) / ( (float)iterationNo / (float)dimensionality) );
//    currentDimNo = (int)ceilf( (float)(iterationNo - it_no) / ( (float)iterationNo / (float)dimensionality) );
//    Borland doesn't like ceilf. So We leave it as ceil which should take a double..
    currentDimNo = currentDimNo < 2 ? 2 : currentDimNo;

    switch(drt){
	case STARBURST:
	    if(currentDimNo > 2)
		dimFactors[currentDimNo-1] = 0;
	    break;
	case GRADUAL_SERIAL:
	    dimFactors[currentDimNo-1] -= reductionFactor / ((float)iterationNo / (float)dimensionality);
	    break;
	default :
	    cerr << "ReduceDimensionality unknown DimReductionType : " << drt << endl;
    }
    // and then make sure that dimFactors 0, 1 are 1 and that none are less than 0
    for(int i=0; i < 2 && i < dimensionality; ++i)
	dimFactors[i] = 1.0;
    for(int i=2; i < dimensionality; ++i)
	dimFactors[i] = dimFactors[i] > 0 ? dimFactors[i] : 0;
}

float DistanceMapper::effectiveDimensionality()
{
  float dim = 0;
  for(int i=0; i < dimensionality; ++i)
    dim += dimFactors[i];
  return(dim);
}

void DistanceMapper::resetDimFactors(){
    delete dimFactors;
    dimFactors = new float[dimensionality];
    for(int i=0; i < dimensionality; ++i)
	dimFactors[i] = 1.0;
    return;
}

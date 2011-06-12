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
#include <qthread.h>
#include <math.h>
#ifdef Q_OS_MACX
#include <limits.h>
#include <float.h>
#define MINFLOAT (FLT_MIN)
#else
#include <values.h>
#endif
#include <qmutex.h>
#include <qapplication.h>
#include <vector>
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

}

dpoint::dpoint(int i, int dimensions, int compNo){
  index = i;
  stress = 0;
  dimNo = dimensions;
  coordinates = new float[dimNo];
  forceVectors = new float[dimNo];
  if(compNo < 1){ compNo = 1; }
  components = new componentVector*[compNo];     // where do I add these, I'm not sure.. we'll have to work it out later.. 
  memset((void*)components, 0, sizeof(componentVector*) * compNo); 
  componentSize = compNo;
  componentNo = 0;
  //  coordinates.resize(dimensions);
  // forceVectors.resize(dimensions);
  for(int i=0; i < dimNo; i++){
    coordinates[i] = 0;
    forceVectors[i] = 0;
  }
  
}

dpoint::~dpoint(){
  for(int i=0; i < componentNo; i++){
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
  if(!includeComponents)
    return(np);
  for(int i=0; i < componentNo; ++i)
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
    p->stress = stress;
    for(int i=0; i < componentNo; ++i){
	p->components[i]->attractive = components[i]->attractive;
	p->components[i]->forceNo = components[i]->forceNo;   /// But this seems very inadvisable.
	memcpy(p->components[i]->forces, components[i]->forces, sizeof(float) * components[i]->forceNo);
    }
}


// Since we don't need to add any more memory, we can just decrease the number that tell us how
// many dimensions we are using. For the dpoint that is just changing the dimNo itself. For the
// componentVectors, all that is necessary is to change forceNo.
int dpoint::shrinkDimNo(uint s){
    if((uint)dimNo - s < 2){
	cerr << "Attempt to shrink dpoint dimNo to " << (uint)dimNo - s << "  foiled. " << endl;
	return(0);
    }
    dimNo = (uint)dimNo - s;
    for(int i = 0; i < componentNo; ++i){
	components[i]->forceNo = dimNo;
    }
    return(dimNo);
}

void dpoint::addComponent(componentVector* cv){
  //void dpoint::addComponent(float* f, int n, bool a){
  //  cout << "point with address : " << (long)this << "  adding component vector component No " << componentNo << "  size : " << componentSize << endl;
    if(componentNo < componentSize){
	delete components[componentNo];  // This should be safe in C++ as long as the pointer is set to 0.
	components[componentNo] = cv;
	componentNo++;
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
float dpoint::adjustVectors(dpoint* p, float d, float* dimFactors, bool linear){
  if(dimNo != p->dimNo){
    cerr << "point coordinate mismatch size thingy : " << endl;
    return(stress);
  }
  // we can speed up the process by making coordDists a member, so we don't have to new it every time (bigger memory use though)
  float* coordDists = new float[dimNo];

  float D = 0;                 // the actual euclidean distance.. 
  for(int i=0; i < dimNo; i++){
    coordDists[i] = dimFactors[i] * (p->coordinates[i] - coordinates[i]);
    if(coordDists[i] == 0){                // disaster.. 
      coordDists[i] = MINFLOAT;                   // it's cheating, to avoid divide by 0 for a D of 0. (maybe better below).
    }
    D += (coordDists[i] * coordDists[i]);
  }
  D = sqrt(D);   // alleluliahh

  float delta;
  if(linear){
    delta = (D- d); // linear  by the fold difference.. 
  }else{
    delta = (D- d) * fabs(log(D/d));    // scale by the fold difference.. 
  }

                                  // adjust the force vectors by some measure
  stress += fabs(delta);     // the absolute amount of stress on the point.. 
  float* compForces = new float[dimNo];
  for(int i=0; i < dimNo; i++ && D > 0){
    if(fabs(delta) > MINFLOAT){
      forceVectors[i] += (delta * coordDists[i])/D;
      compForces[i] = (delta * coordDists[i])/D;   // Squaring seems to get us out of normal range of values.. bugger.  
    }else{
      compForces[i] = 0;
    }
  }
  addComponent(new componentVector(compForces, dimNo, (delta > 0)));
  delete []coordDists;
  return(stress);     // not really useful as it's an intermediate value.. 
}

float dpoint::move(float forceMultiplier){
  float distanceMoved = 0;
  for(int i=0; i < dimNo; i++){
    coordinates[i] += forceMultiplier * forceVectors[i];
    distanceMoved += ((forceMultiplier * forceVectors[i]) * (forceMultiplier * forceVectors[i]));
  }
  return(sqrt(distanceMoved));     // i.e. the euclidean distance.. 
}

void dpoint::resetForces(){              /// THIS GIVES RISE TO A MEMORY LEAK IF NOT USED WITH EXTREME CARE AS IT LEAVES LOTS OF HANGING POINTERS
  stress = 0;
  for(int i=0; i < dimNo; i++){
    forceVectors[i] = 0;
  }
  componentNo = 0;        // componentSize still holds what has been assigned.
}

void dpoint::setPos(vector<float> coords){
  position = coords;
}

DistanceMapper::DistanceMapper(vector<int> expI, vector<vector<float> > d, QMutex* mutx, vector<vector<dpoint*> >* prntPoints, QObject* prnt, vector<stressInfo>* stressLevels, DimReductionType drt){
  //cout << "beginning of distance mapper constructor " << endl;
  calculating = false;
  expts = expI;
  distances = d;
  parent = prnt;
  parentPoints = prntPoints;
  pointMutex = mutx;
  errors = stressLevels;

  iterationNo = 0;  // this means nothing happens unless setDim is called at some point.
  dimensionality = 4;  // actually set by setDim, without which we do nothing
  currentDimNo = dimensionality;

  // set the dimFactors to 1.
  // Note this will crash if dimensionality is less than 0. 
  dimFactors = 0;
  dimReductionType = drt;
  drtMap.insert(make_pair((int)STARBURST, STARBURST));
  drtMap.insert(make_pair((int)GRADUAL_SERIAL, GRADUAL_SERIAL));
  drtMap.insert(make_pair((int)GRADUAL_PARALLEL, GRADUAL_PARALLEL));
  resetDimFactors();
  

  srand(time(0));
  moveFactor = 0.005;      // just keep it below 1,, above one will lead to strange behaviour.. 
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
  // though that really is overcounting by a fair stretch as we count in both directions.. 
  points.reserve(expts.size());
  for(uint i=0; i < expts.size(); i++){
    points.push_back(new dpoint(expts[i], dimensionality, expts.size()));    // 2 dimensional coordinates.. i.e. map to a flat surface .. 
  }
  // Points are given random positions in a 100^n sized space
  initialisePoints();
}

DistanceMapper::~DistanceMapper(){
  cout << "destryoing the DistanceMapper object, hoho yeahh " << endl;
  delete dimFactors;
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
  for(uint i=0; i < expts.size(); i++){
    float stress = 0;
    for(uint j=0; j < expts.size(); j++){      // crash if the dimensions of the distances are not good.. what the hell though..
      if(i != j){
	stress = points[i]->adjustVectors(points[j], distances[i][j], dimFactors, linear);
      }
    }
    totalStress += stress;
  }
  return totalStress;
}

float DistanceMapper::adjustGridVectors(bool linear){
  float stress = 0;
  for(unsigned int i=0; i < gridPoints.size(); ++i){
    for(unsigned int j=0; j < points.size(); ++j)
      stress += gridPoints[i]->adjustVectors(points[j], gridDistances[i][j], dimFactors, linear);
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
  
  for(int i=0; i < iterationNo; ++i){
      float stress = adjustVectors(linear);
      pointMutex->lock();
      (*errors)[i].setStress(dimensionality, dimFactors, currentDimNo, stress);
      pointMutex->unlock();
      // update the parent..
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
  cout << "distanceMapper is done .." << endl;
  calculating = false; 
}

void DistanceMapper::reInitialise(){
    for(uint i=0; i < points.size(); ++i)
	delete points[i];
    points.resize(0);
    for(uint i=0; i < expts.size(); i++)
	points.push_back(new dpoint(expts[i], dimensionality, expts.size()));    // 2 dimensional coordinates.. i.e. map to a flat surface .. 
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

void DistanceMapper::initialisePoints(){
  calculating = true;
  for(uint i=0; i < points.size(); i++){
    if(initialPoints.size() == points.size())
      points[i]->setPos(initialPoints[i]);
    for(int j=0; j < points[i]->dimNo; j++){
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
  // the position at a given point.
  vector<unsigned int> g_pos(min_v.size(), 0);
  vector<unsigned int> g_pos_counters(min_v.size(), 0);
  gridPoints.resize(grid_size);
  gridDistances.resize(grid_size);
  gridPoints[0] = createGridPoint(pno, g_pos, min_v, max_v);
  for(unsigned int i=1; i < gridPoints.size(); ++i){
    g_pos[0] = i % pno;
    for(unsigned int j=1; j < g_pos.size(); ++j){
      if(!g_pos[j-1]){
	++g_pos_counters[j];
	g_pos[j] = g_pos_counters[j] % pno;
      }
    }
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
      if(i + inc < gridPoints.size())
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
  dpoint* point = new dpoint(0, dimensionality, expts.size());
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


void DistanceMapper::resetDimFactors(){
    delete dimFactors;
    dimFactors = new float[dimensionality];
    for(int i=0; i < dimensionality; ++i)
	dimFactors[i] = 1.0;
    return;
}

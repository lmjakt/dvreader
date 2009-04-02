//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
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
  //  cout << "dpoint destructor .. " << endl;
  //cout << "\t\tdpoint destructor comp no is : " << componentNo << endl;
  for(int i=0; i < componentNo; i++){
    delete components[i];
  }
  delete []components;  // should just delete the pointers, not the thing they point to (which are other pointers.. )
  delete []forceVectors;
  delete []coordinates;
}

dpoint* dpoint::copy(){
  // make a new thingy and copy it.. 
  dpoint* np = new dpoint(index, dimNo, componentSize);        // note I don't need to copy the components, as these will be evaluated.. 
  memcpy(np->coordinates, coordinates, sizeof(float)*dimNo);
  memcpy(np->forceVectors, forceVectors, sizeof(float)*dimNo);
  return(np);
}

void dpoint::assignTo(dpoint* point){      // makes point equal to this (copies the things..)
    // first the coordinates and the forceVectors 
    // one value per dimensions..
    if(point->dimNo != dimNo){
	delete point->coordinates;
	delete point->forceVectors;
	point->dimNo = dimNo;
	point->coordinates = new float[dimNo];
	point->forceVectors = new float[dimNo];
    }
    memcpy((void*)point->coordinates, coordinates, sizeof(float) * dimNo);
    memcpy((void*)point->forceVectors, forceVectors, sizeof(float) * dimNo);
    
    // and the memory for the component vectors...
    if(point->componentNo != componentNo || point->componentSize != componentSize){
	for(int i=0; i < point->componentNo; ++i)
	    delete point->components[i];
	delete point->components;
	point->componentSize = componentSize;
	point->componentNo = componentNo;
	point->components = new componentVector*[componentSize];
	for(int i=0; i < componentNo; ++i){
	    point->components[i] = new componentVector();
	    point->components[i]->assignFrom(components[i]);
	}
    }else{
	for(int i=0; i < componentNo; i++){
	    point->components[i]->assignFrom(components[i]);
	}
    }
    point->index = index;
    point->stress = stress;
}

void dpoint::addComponent(componentVector* cv){
    //void dpoint::addComponent(float* f, int n, bool a){
    if(componentNo < componentSize){
	if(components[componentNo]){
	    delete components[componentNo];
	}
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

float dpoint::adjustVectors(dpoint* p, float d, bool linear){
    if(dimNo != p->dimNo){
	cerr << "point coordinate mismatch size thingy : " << endl;
	return(stress);
    }
    // we can speed up the process by making coordDists a member, so we don't have to new it every time (bigger memory use though)
    float* coordDists = new float[dimNo];
    
    float D = 0;                 // the actual euclidean distance.. 
    for(int i=0; i < dimNo; i++){
	coordDists[i] = p->coordinates[i] - coordinates[i];
	if(coordDists[i] == 0){                // disaster.. 
	    coordDists[i] = MINFLOAT;                   // it's cheating, but it will cause some sort of movement away from each other (or towards.)
	    // maybe we don't have to initialise anything then.. we can just cheat. and set everything
	    // to 0 in the beginning.. -- giving us reproducible results as well. hmm 
	    cerr << "cheating, setting coorDists to  " << MINFLOAT <<  "  for : " << i << "   for point : " << index << "  compared to : " << p->index << endl;
	}
	D += (coordDists[i] * coordDists[i]);
	// I know that the above looks really very clumsy, but it is in fact much faster than using the pow function
    }
    D = sqrt(D);   // alleluliahh
    // how about ...
    float delta;
    if(linear){
	delta = (D- d); // linear  by the fold difference.. 
    }else{
	delta = (D- d) * fabs(log(D/d));    // scale by the fold difference.. 
    }
    // adjust the force vectors by some measure
    stress += fabs(delta);     // the absolute amount of stress on the point.. 
    //stress += (delta*delta);     // the absolute amount of stress on the point.. 
    //// multiply each coordinate distance by (delta / D) and add to the force Vectors..
    /// 
    ///  if delta is negative (the points are too close to each other,, then the coordinate distances will be multiplied by 
    ///  a negative number.. and will tend to increase the distance between the two points.. I think.. hmm.
    //cout << "Force Vectors increment  : " << endl;

    float* compForces;
    if(componentNo < componentSize && components[componentNo]){
	compForces = components[componentNo]->forces;
	components[componentNo]->attractive = (delta > 0);
	componentNo++;
    }else{
	compForces = new float[dimNo];
	addComponent(new componentVector(compForces, dimNo, (delta > 0)));
    }

    for(int i=0; i < dimNo; i++){
	if(fabs(delta) > MINFLOAT){
	    forceVectors[i] += (delta * coordDists[i])/D;
	    compForces[i] = (delta * coordDists[i])/D;   // Squaring seems to get us out of normal range of values.. bugger.  
	}else{
	    compForces[i] = 0;
	}
    }
//    addComponent(new componentVector(compForces, dimNo, (delta > 0)));
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
    for(int i=0; i < componentNo; ++i){
	if(components[i]){
	    components[i]->reset();  // this may not be necessary.. as they get set in the adjust
	}
    }
    componentNo = 0;        // that way we don't have to allocate any new memory, and we should see a large speedup..
    //  for(int i=0; i < dimNo; i++){
    //  components[i].forces.resize(0);  // ??
    //}
    //components.resize(0);
}



DistanceMapper::DistanceMapper(vector<int> expI, vector<vector<float> > d, int dims,  QMutex* mutx, vector<dpoint*>* prntPoints, QObject* prnt, bool* drawPoints){
    //cout << "beginning of distance mapper constructor " << endl;
    calculating = false;
    expts = expI;
    distances = d;
    dimensionality = dims;
    parent = prnt;
    parentPoints = prntPoints;
    pointMutex = mutx;
    reDrawPoints = drawPoints;
    
    srand(time(0));
    //moveFactor = 0.005;      // just keep it below 1,, above one will lead to strange behaviour.. 
    moveFactor = 0.005;      // just keep it below 1,, above one will lead to strange behaviour.. 
    // make sure that the distances are all the same.. 
    if(expts.size() != distances.size()){
	cerr << "bugger everytyhing is going to die man.. " << endl;
	cerr << "distances size is : " << distances.size() << endl
	     << "experiments size is : " << expts.size() << endl;
    }
    // count the total distance in the thingy.. as counted by the program..
    totalDistance = 0;
    for(int i=0; i < distances.size(); i++){
	for(int j=0; j < distances[i].size(); j++){
	    totalDistance += distances[i][j];
	}
    }
    // though that really is overcounting by a fair stretch as we count in both directions.. 
    points.reserve(expts.size());
    parentPoints->resize(0);
    parentPoints->reserve(expts.size());
    for(int i=0; i < expts.size(); i++){
	points.push_back(new dpoint(expts[i], dimensionality, expts.size()));    // 2 dimensional coordinates.. i.e. map to a flat surface .. 
	parentPoints->push_back(new dpoint(expts[i], dimensionality, expts.size()));    // 2 dimensional coordinates.. i.e. map to a flat surface .. 
    }
    initialisePoints();
    // these are all now initialised to the origin which is sort of alright.. I suppose... hooo yeahh.
    //cout << "end of distance mapper constructor " << endl;
}

DistanceMapper::~DistanceMapper(){
  cout << "destryoing the DistanceMapper object, hoho yeahh " << endl;
}

void DistanceMapper::restart(){
  wait();   // wait for the thread to finish, this could end up taking a long time..
  calculating = true;
  initialisePoints();  // ok lalal 
  //run();    // and start ..
}

void DistanceMapper::updatePosition(int i, float x, float y){
//  wait();
  //  calculating = true;  -- Not sure about this change.. 
  if(points.size() > i){
    if(points[i]->dimNo > 1){
      points[i]->coordinates[0] = x;
      points[i]->coordinates[1] = y;
    }
  }
  resetPoints();
  adjustVectors();
  pointMutex->lock();
  for(int i=0; i < points.size(); ++i){
      points[i]->assignTo((*parentPoints)[i]);
  }
//  parentPoints->push_back(points);     // this should cause a copy if I understand correctly..
  pointMutex->unlock();
//  clonePoints();                       // necessary to make sure we have new points.. 
  //run();
}

float DistanceMapper::adjustVectors(bool linear){
  // Looks like we could optimise this one,, in some way.. 
  float totalStress = 0;
  for(int i=0; i < expts.size(); i++){
    float stress;
    for(int j=0; j < expts.size(); j++){      // crash if the dimensions of the distances are not good.. what the hell though..
      if(i != j){
	stress = points[i]->adjustVectors(points[j], distances[i][j], linear);
      }
    }
    totalStress += stress;
  }
  return totalStress;
}

float DistanceMapper::movePoints(){
  float movedDistance = 0;
  for(int i=0; i < points.size(); i++){
    movedDistance += points[i]->move(moveFactor);
  }
  return(movedDistance);
}

void DistanceMapper::resetPoints(){
  for(int i=0; i < points.size(); i++){
    points[i]->resetForces();
  }
}

void DistanceMapper::run(){
  calculating = true;
  int generationCounter = 0;
  bool keepOnGoing = true;
  bool linear = true;
  int halfPeriod = 250;
  while(keepOnGoing){     // initially just keep running.. until we add some control factors into the situation..
    int half = generationCounter / halfPeriod;

    resetPoints();
    float stress = adjustVectors(linear);
    // update the parent..
    pointMutex->lock();
    /// WARNING THIS ASSUMES THAT PARENT POINTS IS THE SAME SIZE. IF THIS IS NOT THE CASE THEN WE'LL BE SCREWED.. /////
    for(uint i=0; i < parentPoints->size(); ++i){
	points[i]->assignTo((*parentPoints)[i]);
    }
    (*reDrawPoints) = true;
//    parentPoints->push_back(points);     // this should cause a copy if I understand correctly..
    pointMutex->unlock();
    cout << "DistanceMapper generation : " << generationCounter++ << " stress : " << stress  // << "  moved : " << distanceMoved
	 << "   total distance : " << totalDistance << "   Linear is : " << linear << endl;

//    clonePoints();     // need to make sure we don't touch the same vectors again.. 
    float distanceMoved = movePoints();

    if(generationCounter >= halfPeriod*2){ keepOnGoing = false; }
    //   if(distanceMoved < 0.1 || generationCounter > 200){ keepOnGoing = false; }
  }
  calculating = false; 
}

void DistanceMapper::initialisePoints(){
  calculating = true;
  for(int i=0; i < points.size(); i++){
    for(int j=0; j < points[i]->dimNo; j++){
      points[i]->coordinates[j] = float(100) * rand()/RAND_MAX;
    }
  }
} 


void DistanceMapper::clonePoints(){
  // because we are handing over control of the points to the owner, we need to have some new points to 
  // manipulate. Because we are using pointers for everything, we will need to copy the thingy.. probably the best way 
  // would be to provide a copy constructor.. -- using = ,, but hmm,, I think a copy command might be better.. 
  for(int i=0; i < points.size(); i++){
//     cout << "clonePoints point " << i << "  coordinates and forces " << endl;
//     for(int j=0; j < points[i]->dimNo; j++){
//       cout << "\t" << j << "   coord " << points[i]->coordinates[j] << "   forceVector : " << points[i]->forceVectors[j] << endl;
//     }
    points[i] = points[i]->copy();    // this doesn't copy the componentVectors, nor the stress, but should be enough.. Note this is could easily lead to memory leaks..
//     cout << "After Cloning .. " << endl;
//     for(int j=0; j < points[i]->dimNo; j++){
//       cout << "\t" << j << "   coord " << points[i]->coordinates[j] << "   forceVector : " << points[i]->forceVectors[j] << endl;
//     }
  }
}

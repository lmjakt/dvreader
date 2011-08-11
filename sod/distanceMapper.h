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

#ifndef DISTANCEMAPPER_H
#define DISTANCEMAPPER_H

#include <qthread.h>
#include <vector>
#include <qmutex.h>
#include <iostream>
#include <string.h>
#include <map>
#include <set>


struct stressInfo {
  unsigned int dimNo;
  unsigned int activeDimNo;
  std::vector<float> dimFactors;
  float stress;

  stressInfo(){
    dimNo = activeDimNo = 0;
    stress = 0;
  }
  void setStress(int dim_no, float* df, uint ad, float s){
    dimNo = dim_no;
    activeDimNo = ad;
    dimFactors.resize(dimNo);
    for(uint i=0; i < dimNo; ++i)
      dimFactors[i] = df[i];
    stress = s;
  }
  float dimensionality(){
    float d=0;
    for(unsigned int i=0; i < dimFactors.size(); ++i)
      d += dimFactors[i];
    return(d);
  }
  float currentDF(){
    if(activeDimNo - 1 > 0)
      return(dimFactors[activeDimNo - 1]);
    return(1.0);
  }
};

struct componentVector {
  bool attractive;     // is it an attractor or repulsor..
  float* forces;
  int forceNo;        // this should have been called dimNo for dimensionNo, I may rename later .. 
  
  componentVector(){};   // empty constructor
  componentVector(unsigned int dimNo){
    attractive = true;
    forceNo = dimNo;
    forces = new float[dimNo];
    memset((void*)forces, 0, sizeof(float) * dimNo);
  }
  componentVector(std::vector<float> f, bool a){
    attractive = a;
    forces = new float[f.size()];
    forceNo = f.size();
    for(int i=0; i < forceNo; i++){
      forces[i] = f[i];
    }
  }
  componentVector(float* f, int n, bool a){
    attractive = a;
    forces = f;
    forceNo = n;
  }
  ~componentVector(){
    delete []forces;
  }
  componentVector* copy(){
    componentVector* cp = new componentVector();
    cp->attractive = attractive;
    cp->forceNo = forceNo;
    cp->forces = new float[forceNo];
    memcpy( (void*)cp->forces, forces, sizeof(float) * forceNo);
    return(cp);
  }
};

struct dpoint {
  
  float* coordinates;
  float* forceVectors;
  //  float forceMagnitudeSum;
  unsigned int dimNo;       // the no of dimensions. --  we have one value for each forceVector.. 
  
  componentVector** components;
  unsigned int componentNo;
  unsigned int componentSize;   // memory allocated for components.
  int index;                    // some kind of identifier..
  float stress;                 // total amount of stress (absolute).. 
  
  // an optional position (when the starting dimensionality is reasonably small)
  std::vector<float> position;  
  // an optional vector set for grid points (passive points representing the
  // distortion of n-dimensional space).
  std::vector<dpoint*> neighbors;
  
  // optional for various purposes.. 
  std::set<unsigned int> neighbor_indices;  // should probably get rid of the neighbors vector
  
  dpoint();  
  dpoint(int i, unsigned int dimensions, unsigned int compNo);
  ~dpoint();

  // given another point, with an optimal distance of d
  // adjust the vectors and return a measure of force in the system.. 
  float adjustVectors(dpoint* p, float d, float* dimFactors, bool linear, unsigned int comp_index);
  // the fast method does not calculate any componentVectors. The buffer should be of dimFactors length (to avoid creating a new one)
  float adjustVectorsFast(dpoint* p, float d, float* dimFactors, float* buffer);  
  // move the point, return the euclidean distance moved
  float move(float forceMultiplier);              
  // set the stress and force vectors to be 0.. 
  void resetForces();
  void setPos(std::vector<float> coords);
  dpoint* copy(bool includeComponents=false);
  void assignValues(dpoint* p);  // assign values to p from this   
  int shrinkDimNo(uint s);   // shrinks the number of dimensions by s.
  
private :
  //void addComponent(float* f, int n, bool a);
  void addComponent(componentVector* cv);    // check how much memory we have, expand if necessary, otherwise, just add.. 
  
};


// A class that spawns a thread that attempts to map a set of objects in 2d based
// on all agains all distances.. -- whilst calculating the stress in the system.. 

class DistanceMapper : public QThread
{
 public :
  enum DimReductionType {
    STARBURST, GRADUAL_SERIAL, GRADUAL_PARALLEL
  };
  
  DistanceMapper(std::vector<int> expI, std::vector<std::vector<float> > d, QMutex* mutx, 
		 std::vector<std::vector<dpoint*> >* prntPoints, QObject* prnt, 
		 std::vector<stressInfo>* stressLevels, DimReductionType drt, bool rememberComponents);
  ~DistanceMapper();  // don't know how much I'll need for these..
  void restart();     // restart, but this is deprecated at the moment. Avoid this.
  void updatePosition(int i, float x, float y);  
  void reInitialise();
  void setDim(int dim, int iter, int drt);
  void setIter(int iter);
  void setThreadNumber(unsigned int tno);
  void setMoveFactor(float mf);
  void setSubset(unsigned int subset_size);
  void useSubSet(bool useSub);
  void setInitialPoints(std::vector<std::vector<float> > i_points, unsigned int grid_points = 0);
  void setUpdateInterval(unsigned int ui);
  void makeTriangles();  // set's up neighbour_indices to make a 2D mesh connecting points??
  std::vector<dpoint*> grid();

  bool calculating; 
  bool initOK;        // so we can check if it initialised ok.. 
  private :
  std::vector<int> expts;    // in this case each object represents an experiment, but it could be anything, -- a gene even. !!
  std::vector<std::vector<float> > distances;  // the optimal distances. 

  // For large data sets, we can compare each point to a set of closest neighbours rather than to
  // all members. This should work most of the time, but may occasionally fail dramatically.. 
  std::vector<unsigned int*> compare_subsets;
  unsigned int subset_length;
  bool use_subset;
  
  // starting points, unfortunately get stored many times, as we keep a copy in all of the points
  std::vector<std::vector<float> > initialPoints; 
  std::vector<dpoint*> points;              // the points representing the objects (usually experiments.. )..  
  std::vector<std::vector<dpoint*> >* parentPoints;
  std::vector<stressInfo>* errors;

  // points representing a grid distorted by points (like points, but do not affect the movement)
  std::vector<dpoint*> gridPoints;      // deleted by the viewer.
  std::vector<std::vector<float> > gridDistances; // initial distances to points.

  QObject* parent;                 // for updating information.. 
  QMutex* pointMutex;

  float moveFactor;               // how much do we move a point by. -- i.e. how much do we multiply the forceVectors by to get a 
                                  // to move distance.. 
  unsigned int thread_no;         // the number of threads to use when mapping.
  bool rememberComponentVectors; // whether or not to create coordinate vectors
  unsigned int update_interval;   // allow the updating to be done for every n iterations.. 
  float totalDistance;
  int dimensionality;
  int currentDimNo;
  int iterationNo;
  float* dimFactors;   // used for squeezing dimensions.. should have the size of dimensionality.. 
  DimReductionType dimReductionType;
  std::map<int, DimReductionType> drtMap;  // This seems a bit silly but I dont think I can do (DimReductType)int .. 
  // some functions..

  float adjustVectors(bool linear=true);          // adjust the Vectors in the points.. returns the total force
  float adjustGridVectors(bool linear=true);
  float movePoints(std::vector<dpoint*>& pnts);             // move the points.. and reset force vectors.. -- return distance moved
  void resetPoints(std::vector<dpoint*>& pnts);             // just go through and reset the points.. 
  void initialisePoints();       // find some nice random positions for the points..
  void initialiseGrid(unsigned int pno, std::vector<std::vector<float> >& i_points);
  void createGridPoints(unsigned int pno, std::vector<float> min_v, std::vector<float> max_v);
  dpoint* createGridPoint(unsigned int pno, std::vector<unsigned int>& g_pos, 
			  std::vector<float>& min_v, std::vector<float>& max_v);
  void updateParentPoints();
  void clonePoints();
  void reduceDimensionality(DimReductionType drt, int it_no);   // adjust dimFactors by some means..
  float effectiveDimensionality();
  void resetDimFactors();        // deletes dimFactors and then resets them to dimensionality.. 

  protected :
    void run();                 // the main function, essentially iterates through initialise points and 
};

#endif

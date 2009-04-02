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

#ifndef KCLUSTERPROCESS_H
#define KCLUSTERPROCESS_H

#include <QThread>
//#include <qthread.h>
#include <qmutex.h>
#include <iostream>
#include <vector>           // only for initialisation of clusters.. which doesn't take much processing. 
//#include <set>
#include "../dataStructs.h"
#include <qcolor.h>

//using namespace std;

struct Cluster {            // just some data that we might want to return..
    std::vector<simple_drop> members;    // the members of the id..   (which will inevitably end up being a copy.
    std::vector<float> distances;       // distances from the center.. (gives us an idea of the tightness of the cluster)
    std::vector<float> center;
    float maxDistance;                  // for convenience..
    QColor color;                       // convenient to be able to set for drawing stuff.. 

    // we also have some variables for modelling.. 

    std::vector<float> xline;
    std::vector<float> yline;
    std::vector<float> zline;

    std::vector<float> gm_x;  // the gaussian model..
    std::vector<float> gm_y;
    std::vector<float> gm_z;  // we don't really need to store these, but might as well

    // some variables that define the model
    float xMean, xVar, xSum;
    float yMean, yVar, ySum;
    float zMean, zVar, zSum;
    float xbg, ybg, zbg;  // backgrounds .. 
    bool gm_model;   // whether or not the model has been defined.. 

    Cluster(){
	maxDistance = 0;
	color = QColor(255, 255, 255);
	gm_model = false;
    }
    Cluster(float md){
	maxDistance = md;
	color = QColor(255, 255, 255);
	gm_model = false;
    }
    // we don't need much more of a constructor since we'll have to fill in the values anyway, and since
    // this is a simple struct we can just stick it in there.. 

    // ok, this is complicating issues a bit, adding lots of functions to this particular struct.. a little too much perhaps,,
    // but..
    void makeGaussianModel();   // sets some parameters for a gaussian model of the drop..
    std::vector<float> centralLine(simple_drop& dr, int dim); 

};

class KClusterProcess : public QThread
{
  public :
    KClusterProcess(uint cNo, std::vector<simple_drop>& data);
  ~KClusterProcess();
  std::vector<Cluster> return_clusters();   // a way to give the information to those who want it.. 
  // some things that I may need to access. Ofcourse, I should make accessor functions, but that's just extra work. 

  //bool localNorm;            // use local or global normalisation.. 


  protected :
    void run();

  private :
      
      int k;                     // the number of clusters, equal to cNo
  uint N;                     // the dimensionality of the space, equal to the exptNo
  uint dropNumber;            // the number of drops.. 
  
  float** points;            // the data points,, first dimension has geneNo points, second has N points. 
  float** centers;           // the cluster centers.. first dimension has k points, second has N points.
  
  uint* clusterSizes;         // the sizes of the clusters,, 
  float* maxDistances;      // max distance for each cluster... 
  uint** clusters;            // the members of the clusters..  
  uint* probeIndices;         // the probe Indices clustered.. -so a total of geneNo probeIndices..
  std::vector<simple_drop> original_drops;
  bool someEmpty;            // true if some of the clusters are empty.. 
  uint* membership;           // the membership,, the indices are from probeIndices

  float* min;
  float* max;
  float* centerDistances;   // optional. maybe later I will store the distances from the centers.
  int minChanges;           // the minimum number of changes. set to geneNo initiallay
  int noImprovementCount;       // the number of iterations without an improvement..   

  void initialiseCenters(float* min, float* max);       // length is N anyway... 
  void reallocateEmptyCenters();                        // if we have empty centers, then reallocate these.. 
  void normalise(float* v, int s);
  int allocate();
  void calculateCenters();
  float euclidean(float* v1, float* v2, int s);

  // I am not sure yet. but I think I may need some methods for sorting.. so let's implement a quicksort..
  void swapElements(int* a, int* b);
  int divideArray(int* a, float* v, int left, int right);
  void quickSort(int* indices, float* values, int left, int right);

  // and then the data I need to work on this..

  
  //QMutex* clusterMutex;      // for changing the parentClusters.. 
  //set<void*>* parentClusters; // to pass on myself in the afterlife..

};

#endif

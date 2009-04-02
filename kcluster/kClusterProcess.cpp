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

#include "kClusterProcess.h"
#include "../stat/stat.h"
#include <vector>
#include <set>
#include <qthread.h>
#include <iostream>
#include <qmutex.h>                 // for changing the variable thingy.. 
#include <time.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

void Cluster::makeGaussianModel(){
    // first try to work out the sigmas in the different directions..
    // in order to do this, go through each drop and get the central line in each dimension.
    // have a look at these and then make some sense of them..
    
    // first we need to know the dimensions of the drop volumes. unfortunately 
    // we have to have a look at the first member and assume that things will be the same..
    
    //dis distribution;  // so we can return this. Not sure if this is actually what I want to return, but..
    if(!members.size()){
	cerr << "Cluster makeGaussianModel, no members to calculat from" << endl;
	return;
    }
    int r = members[0].radius;
    int d = r * 2 + 1;    // the diameter..
    if(center.size() != d * d * d){
	cerr << "Center size is not : " << d * d * d << " but rather " << center.size() << endl;
	return;
    }
    
    vector<float> xMeans(members.size());
    vector<float> xVars(members.size());
    vector<float> yMeans(members.size());
    vector<float> yVars(members.size());
    vector<float> zMeans(members.size());
    vector<float> zVars(members.size());

    vector<vector<float> > xLines(members.size());
    vector<vector<float> > yLines(members.size());
    vector<vector<float> > zLines(members.size());  // compare models with real life ??
    // probably I won't actually do it, but lets discard stuff later.. 

//    if(!gm_model){
    xbg = ybg = zbg = 0;
    // then assign some lines..
    xline.resize(d);
    yline.resize(d);
    zline.resize(d);
    for(uint i=0; i < xline.size(); i++){
	xline[i] = yline[i] = zline[i] = 0;
    }
//     vector<float> xline(d, 0);
//     vector<float> yline(d, 0);
//     vector<float> zline(d, 0);
    
    // and then we'll go through the members..
    float fd = float(d);
    for(uint i=0; i < members.size(); i++){
	vector<float> xl = centralLine(members[i], 0);
	vector<float> yl = centralLine(members[i], 1);
	vector<float> zl = centralLine(members[i], 2);
	xLines[i] = xl;
	yLines[i] = yl;
	zLines[i] = zl;
	if(xl.size() != xl.size() || yline.size() != yl.size() || zline.size() != zl.size()){
	    cerr << "some problemw with drop number : " << i << endl;
	    return;
	}
	// and let's get the stats ..
	float mean, var;
	dist_stats(mean, var, xl, 0, fd);
	xMeans[i] = mean;
	xVars[i] = var;
	dist_stats(mean, var, yl, 0, fd);
	yMeans[i] = mean;
	yVars[i] = var;
	dist_stats(mean, var, zl, 0, fd);
	zMeans[i] = mean;
	zVars[i] = var;
	for(uint i=0; i < xline.size(); i++){
	    xline[i] += xl[i];
	    yline[i] += yl[i];
	    zline[i] += zl[i];
	}
    }
    gm_model = true;
//    }

    // let's look at the distribution of values for the different values..
    int buckets = 10;
    distribution xMeanDistribution(xMeans, buckets);
    distribution xVarDistribution(xVars, buckets);
    distribution yMeanDistribution(yMeans, buckets);
    distribution yVarDistribution(yVars, buckets);
    distribution zMeanDistribution(zMeans, buckets);
    distribution zVarDistribution(zVars, buckets);

    // and print the distributions, there probably isn't much here, but..

    cout << "X means" << endl;
    xMeanDistribution.printDistribution();
    cout << "X vars" << endl;
    xVarDistribution.printDistribution();

    cout << "Y means" << endl;
    yMeanDistribution.printDistribution();
    cout << "Y vars" << endl;
    yVarDistribution.printDistribution();

    cout << "Z means" << endl;
    zMeanDistribution.printDistribution();
    cout << "Z vars" << endl;
    zVarDistribution.printDistribution();
    
    // if the model can indeed be said to be gaussian, then essentially these three lines should all represent gaussian distributions..
    // for which we can esily calculate the underlying properties..
    // if only I can remember stuff...
    
    // so let's calculate the different values...
//     float xMean, xVar, xSum;
//     float yMean, yVar, ySum;
//     float zMean, zVar, zSum;
    
//     vector<float> xl(xline.size());
//     vector<float> yl(yline.size());
//     vector<float> zl(zline.size());
//     for(uint i=0; i < xline.size(); i++){
// 	xl[i] = xline[i] - xbg;
// 	yl[i] = yline[i] - ybg;
// 	zl[i] = zline[i] - zbg;
//     }
//     // however, since we want to be able to do background subtraction then we'll need to do something.. 

    
    dist_stats(xMean, xVar, xline, 0, float(xline.size()));
    dist_stats(yMean, yVar, yline, 0, float(yline.size()));
    dist_stats(zMean, zVar, zline, 0, float(zline.size()));

    // lets also calculate the means of the standard deviations and the other stuff..
    float xMeansMean = 0;
    float xVarsMean = 0;
    float yMeansMean = 0;
    float yVarsMean = 0;
    float zMeansMean = 0;
    float zVarsMean = 0;
    for(uint i=0; i < xMeans.size(); i++){
	xMeansMean += (xMeans[i] / float(xMeans.size()));
	xVarsMean += (xVars[i] / float(xVars.size()) );
	yMeansMean += (yMeans[i] / float(yMeans.size()) );
	yVarsMean += (yVars[i] / float(yVars.size()) );
	zMeansMean += (zMeans[i] / float(zMeans.size()) );
	zVarsMean += (zVars[i] / float(zVars.size()) );
    }

    // this now gives me a set of values that I can use to make a model.
    // essentially all I need to do is to calculate a background and adjust for this
    vector<float> gm_x(xline.size());
    vector<float> gm_y(yline.size());
    vector<float> gm_z(yline.size());
    
    // and let's make the model using the variance and the mean.. 
    // note that this model will have values between 0 and 1..
    float gm_xSum, gm_ySum, gm_zSum;
    gm_xSum = gm_ySum = gm_zSum = 0;
    xSum = ySum = zSum = 0;
    for(uint i=0; i < xline.size(); i++){
	float v = float(i);
	float xd = (xMean - v) * (xMean - v);
	float yd = (yMean - v) * (yMean - v);
	float zd = (zMean - v) * (zMean - v);
	gm_x[i] = xline[r] * exp(-xd/xVar);
	gm_y[i] = yline[r] * exp(-yd/yVar);
	gm_z[i] = zline[r] *exp(-zd/zVar);
	xSum += (xline[i] - xbg);
	ySum += (yline[i] - ybg);
	zSum += (zline[i] - zbg);
	gm_xSum += gm_x[i];
	gm_ySum += gm_y[i];
	gm_zSum += gm_z[i];
    }
    // Note that the model will be scaled to have a maximum of 1,, -
    // anyway, let's see how this goes..
    cout << "Cluster model : " << endl;
    cout << "X : mean " << xMean << "  var " << xVar << "  Y : mean " << yMean << "  var " << yVar << "   Z : mean " << zMean << "  var " << zVar << endl;
    cout << "X : mean " << xMeansMean << "  var " << xVarsMean << "  Y : mean " << yMeansMean << "  var " << yVarsMean << "   Z : mean " << zMeansMean << "  var " << zVarsMean << endl;
    // and then let's just print out the values..
    for(uint i=0; i < xline.size(); i++){
	cout << i << "\t" << xline[i] << "\t" << gm_x[i] //* (xSum/gm_xSum) + xbg
	     << "\t" << yline[i] << "\t" << gm_y[i] //* (ySum/gm_ySum) + ybg
	     << "\t" << zline[i] << "\t" << gm_z[i]  << endl; //* (zSum/gm_zSum) + zbg << endl;
// 	gm_x[i] = gm_x[i] * (xSum/gm_xSum) + xbg;
// 	gm_y[i] = gm_y[i] * (ySum/gm_ySum) + ybg;
// 	gm_z[i] = gm_z[i] * (zSum/gm_zSum) + zbg;
    }
    // after which we actually want to bring in a background value to work out how things really should look.. 
    // define the background as simply... 
    xbg = (xline.back() + xline[0] - gm_x.back() - gm_x[0])/4.0;
    ybg = (yline.back() + yline[0] - gm_y.back() - gm_y[0])/4.0;
    zbg = (zline.back() + zline[0] - gm_z.back() - gm_z[0])/4.0;

    cout << "Backgrounds are : xbg " << xbg << "  ybg " << ybg << "  zbg " << zbg << endl;

    xbg = xbg > 0 ? xbg : 0;
    ybg = ybg > 0 ? ybg : 0;
    zbg = zbg > 0 ? zbg : 0;
}

vector<float> Cluster::centralLine(simple_drop& dr, int dim){
    int s = dr.radius * 2 + 1;
    int S = s * s;
    vector<float> line(s, 0);
    if(dim > 2)
	return(line);
    if((int)dr.values.size() != S * s){
	cerr << "Cluster::centralLine dr.value.size() is not equal to S * s : " << dr.values.size() << " : " << S * s << endl;
	return(line);
    }
    int x, y, z;
    x = y = z = dr.radius;
    for(int i=0; i < s; ++i){
	switch(dim){
	    case 0 :
		x = i;
		break;
	    case 1 :
		y = i;
		break;
	    case 2 :
		z = i;
	}
	line[i] = dr.values[z * S + y * s + x];
    }
    return(line);
}
    
KClusterProcess::KClusterProcess(uint cNo, vector<simple_drop>& drops){
    // first do a sanity check, make sure that all the dropVolume have the same radius,
    // and that all have got the values filled in 
    original_drops = drops;
    points = 0;
    centers = 0;
    if(!drops.size()){
	cerr << "KClusterProcess, no drops.. " << endl;
	N = 0;
	dropNumber = 0;
	k = 0;
	return;
    }
    N = drops[0].values.size();
    cout << "N  ---> " << N << endl;
    int radius = drops[0].radius;
    int diameter = radius * 2 + 1;
    int volume = diameter * diameter * diameter;
    cout << "set N, radius, diameter and volume" << endl;
    if(N != volume){
	cerr << "volume not equal to N " << endl;
	return;
    }
    for(uint i=0; i < drops.size(); i++){
	if(drops[i].radius != radius or drops[i].values.size() != volume){
	    cerr << "Drop " << i << " does not appear to have the appropriate dimensions" << endl;
	    return;
	}
    }
    cout << "done the sanity checking " << endl;
    dropNumber = drops.size();
    k = cNo;
    if(k > dropNumber){
	k = dropNumber;
    }
    someEmpty = true;                     // we should probably set a flag to give the user an option as to allow empty clusters or not. 

    points = new float*[drops.size()];
    centerDistances = new float[dropNumber];
    membership = new uint[dropNumber];

    // first create a min and max array with N things in them
    min = new float[N];
    max = new float[N];
    for(int i=0; i < N; i++){ min[i] = max[i] = 0; }    // assume no negative numbers, but we'd rather like space to be limited to 0 anyway, so..
    for(uint i=0; i < drops.size(); i++){
	points[i] = new float[N];
	for(uint j=0; j < drops[i].values.size(); j++){
	    points[i][j] = drops[i].values[j];
	    if(points[i][j] > max[j]){ max[j] = points[i][j]; }
	    if(points[i][j] < min[j]){ min[j] = points[i][j]; }
	}
    }
    cout << "Allocated the points " << endl;
    /// at which point we have set the values.
    // anslo need to allocate the centers.. 

    maxDistances = new float[k];
    clusterSizes = new uint[k];
    centers = new float*[k];    // the second dimension gets set in initialiseCenters(min, max). 
    clusters = new uint*[k];
    for(int i=0; i < k; i++){
	clusterSizes[i] = 0;
	clusters[i] = 0;            // this doesn't get set until the end of the process. 
    }
    cout << "allocated the first dimension of the centers " << endl;
    // now the data is all initialised. hopefully it's OK. need to initialise the cluster Centers.
    initialiseCenters(min, max);
    cout << "centers allocated " << endl;
    // delete the temp values..
    minChanges = dropNumber;     // the minimum number of changes made, (to see if we are getting better or worse)
    noImprovementCount = 0;
    cout << "and the constructuur is done " << endl;
}

KClusterProcess::~KClusterProcess(){
  cout << "Deleting a KClusterProcess " << endl;
  for(int i=0; i < dropNumber; i++){
    delete []points[i];
  }
  delete []points;
  
  for(int i=0; i < k; i++){
    delete []clusters[i];
    delete []centers[i];
  }
  delete []clusters;
  delete []centers;
  delete []clusterSizes;
  delete []membership;
  delete []min;
  delete []max;
  delete []centerDistances;
  delete []maxDistances;
}

void KClusterProcess::initialiseCenters(float* min, float* max){
  // assume that the first dimension of the centers has already been allocated.
  srand(time(0));
  for(int i=0; i < k; i++){
    centers[i] = new float[N];
    for(int j=0; j < N; j++){
      centers[i][j] = min[j] + ( (max[j]-min[j]) * ((float)rand()/(float)RAND_MAX) );
    }
  }
}

void KClusterProcess::reallocateEmptyCenters(){
  // order clusters containing more than one member, i.e. not empty by their size,
  // allocate empty clusters to a random member within the largest clusters.. 
  // well, one for each.. 
  //cout << "beginning of reallocateEmptyCenters " << endl;
  // first we need to get a sorted index for the clusters based on the cluster max distances..
  int* tempClusterIndex = new int[k];
  for(int i=0; i < k; i++){ tempClusterIndex[i] = i; }
  //  cout << "just before sorting.. what is going on" << endl;
  quickSort(tempClusterIndex, maxDistances, 0, k-1);   // 0 now contains the smallest.. no good.. we want reverse order.. 
  //cout << "tempclusters index made and sorted " << endl;

  srand(time(0));
  int emptyCount = 0;
  int clusterIterator=0;
  for(int i=0; i < k; i++){
    if(clusterSizes[i] == 0){
      emptyCount++;
      // choose the appropriate cluster..
      while(maxDistances[tempClusterIndex[k-1-(clusterIterator % k)]] == 0){
	clusterIterator++;
      }
      //cout << "i : " << i << "  iterator " << clusterIterator << endl;
      // get a random integer in the range of the size of the chosen cluster..
      int clusterChoice = tempClusterIndex[k-1-(clusterIterator % k)];      // just for clarity, not for speed.. 
      clusterIterator++;
      //cout << "cluster Choice is " << clusterChoice << endl;
      int choice = clusterSizes[clusterChoice] * ((float)(rand()-1)/(float)RAND_MAX);   // will fail if,, rand() = RAND_MAX
      //cout << "And choice is     " << choice << endl;
      //cout << "which turns out to be index " << clusters[clusterChoice][choice] << endl;
      //      centers[i] = points[clusters[clusterChoice][choice]];
      for(int j=0; j < N; j++){
	centers[i][j] = points[clusters[clusterChoice][choice]][j];
	//	centers[i][j] = min[j] + ( (max[j]-min[j]) * ((float)rand()/(float)RAND_MAX) );
      }
    }
  }
  delete []tempClusterIndex;
  //cout << "reallocate Empty Centers : emptyCount is " << emptyCount << endl;
  if(emptyCount == 0){
    someEmpty = false;
  }
}

void KClusterProcess::normalise(float* v, int s){
  // find the mean and std deviation..
  // Note..      SS = sum of squared devieates (squared sum)
  // AND         SS = sum(X^2) - ((sum(X))^2 / N)
  // so          std = sqrt( SS / N-1)
  // which should be reasonably fast to calculate.. (rather than using the std function.. (which I could rewrite)
  float std;
  float mean;
  if(s > 1){
    float xsum = 0;
    float xsquaredSum = 0;
    for(int i=0; i < s; i++){
      xsum += v[i];
      xsquaredSum += (v[i]*v[i]);
    }
    mean = xsum/s;
    float SS = xsquaredSum - (xsum*xsum)/(float)s;
    std = sqrt(SS/(float)(s-1));
    // then go through and  modify
    for(int i=0; i < s; i++){
      v[i] = (v[i]-mean)/std;
    }
  }
}	

float KClusterProcess::euclidean(float* v1, float* v2, int s){
  float e = 0;
  if(s <= 0){
    return(-1);
  }
  for(int i=0; i < s; i++){
    e += ((v1[i] - v2[i]) * (v1[i] - v2[i]));
  }
  return(sqrt(e)/s);
}

void KClusterProcess::run(){
    cout << "start of the run process " << endl;
    int* clusterCounts = new int[k];
    int maxIterations = 50;        // number of useless iterations.,, i.e. without an improvement.. 
    while(someEmpty){
	cout << "someEmpty is still true" << endl;
	while(allocate()){
	    //cout << "Allocate returned true " << endl;
	    calculateCenters();
	    //cout << "Centers calculated " << endl;
	    if(noImprovementCount > maxIterations)
		break;
	}
	// allocate the members in the clusters structure.. 
	// again first dimension already allocated. 
	//    cout << "finished calculating centers and am allocating other stuff" << endl;
	for(int i=0; i < k; i++){
	    if(clusters[i] != 0){
		//cout << "\tcalling delete for cluster " << i << endl;
		delete []clusters[i];
	    }
	    //cout << "\tcalling new for cluster " << i << " size " << clusterSizes[i] << endl;
	    clusters[i] = new uint[clusterSizes[i]];
	    clusterCounts[i] = 0;
	    maxDistances[i] = 0;
	    // cout << "\t\tannd set counts and max distances " << endl;
	}
	//cout << "Set lots of values to 0" << endl;
	for(int i=0; i < dropNumber; i++){
	    // cout << "setting clusters for drop : " << i << endl;
	    clusters[membership[i]][clusterCounts[membership[i]]] = i;
	    clusterCounts[membership[i]]++;
	    if(centerDistances[i] > maxDistances[membership[i]]){
		maxDistances[membership[i]] = centerDistances[i];
	    }
	}
	cout << "calling reallocateEmptyCenters " << endl;
	reallocateEmptyCenters();          // this way I can do some statistics on the thing.. 
    }
    // lock the clusterMutex, then insert our address into the thing..
    delete []clusterCounts;
//    clusterMutex->lock();
//    parentClusters->insert((void*)this);
//    clusterMutex->unlock();
    // and that is then it.
}

int KClusterProcess::allocate(){
  int changes = 0;
  // set all clusterSizes to 0
  for(int i=0; i < k; i++){ clusterSizes[i] = 0; }
  
  for(int i=0; i < dropNumber; i++){
    float minDistance = euclidean(points[i], centers[0], N);
    int minIndex = 0;
    for(int j=0; j < k; j++){
      float d = euclidean(points[i], centers[j], N);
      if(minDistance > d){
	minDistance = d;
	minIndex = j;
      }
    }
//    cout << "allocate minDistance " << minDistance << " for drop : " << i << endl;
    if(minIndex != membership[i]){
      changes++;
      membership[i] = minIndex;
    }
    centerDistances[i] = minDistance;    // even if there is no change in the index, the center may have moved.. !! 
    clusterSizes[minIndex]++;
  }
  cout << "changes : " << changes << endl;
  if(changes < minChanges){
    minChanges = changes;
    noImprovementCount = 0;
  }else{
    noImprovementCount++;
  }
  return(changes);
}

void KClusterProcess::calculateCenters(){
  // set all centers to 0.
  for(int i=0; i < k; i++){
    if(clusterSizes[i]){
      for(int j=0; j < N; j++){
	centers[i][j] = 0;
      }
    }
  }
  // then go through all of the genes and move the centers around..
  for(int i=0; i < dropNumber; i++){
    for(int j=0; j < N; j++){
      centers[membership[i]][j] += points[i][j];
    }
  }
  // and then go through and divide by the cluster sizes...
  for(int i=0; i < k; i++){
    if(clusterSizes[i]){
      for(int j=0; j < N; j++){
	centers[i][j] = centers[i][j]/(float)clusterSizes[i];
      }
    }
    //    cout << "cluster " << i << "   size : " << clusterSizes[i] << endl;
  }
}

void KClusterProcess::swapElements(int* a, int* b){
  int temp = *a;
  *a = *b;
  *b = temp;
}

int KClusterProcess::divideArray(int* a, float* v, int left, int right){
  int mid = left;
  while(1){
    while(right > left && v[a[right]] > v[a[mid]])
      right--;
    while(left < right && v[a[left]] <= v[a[mid]])
      left++;
    if(left == right)
      break;
    swapElements(&a[left], &a[right]);
  }
  if(v[a[mid]] > v[a[left]])
    swapElements(&a[mid], &a[left]);
  return(left);
}

void KClusterProcess::quickSort(int* indices, float* values, int left, int right){
  int split;
  if(left < right){
    split = divideArray(indices, values, left, right);
    quickSort(indices, values, left, split-1);
    quickSort(indices, values, split+1, right);
  }
  return;
}

vector<Cluster> KClusterProcess::return_clusters(){
    vector<Cluster> clust(k);     // we already have a uint** clusters -- i.e. the members of the clusters.. 
    // first make sure that we actually have some clusters..
    if(!centers || !points){
	clust.resize(0);
	return(clust);
    }
    // then assume we have something..
    for(uint i=0; i < k; i++){
	// go through the members of cluster i..
	clust[i].members.reserve(clusterSizes[i]);
	clust[i].distances.reserve(clusterSizes[i]);
	clust[i].center.resize(N);
	for(uint j=0; j < clusterSizes[i]; j++){
	    clust[i].members.push_back(original_drops[clusters[i][j]]);
	    clust[i].distances.push_back(centerDistances[clusters[i][j]]);
	}
	clust[i].maxDistance = maxDistances[i];
	for(int j=0; j < N; j++){
	    clust[i].center[j] = centers[i][j];
	}
    }
    return(clust);
}

#include "NNMapper2.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

NNMapper2::NNMapper2(int maxDistance)
{
  maxD = maxDistance;
  sq_maxD = (maxDistance * maxDistance);
  pointSpace = 0;
  perimeterSpace = 0;
}

NNMapper2::~NNMapper2()
{
  delete pointSpace;
  delete perimeterSpace;
}

void NNMapper2::setMaxDistance(int maxDistance){
  maxD = maxDistance;
  sq_maxD = (maxDistance * maxDistance);
}  

void NNMapper2::mapPoints(vector<threeDPoint>& Points, vector<unsigned int> pointsIndex, vector<Perimeter>& Nuclei)
{
  // this will set up points as well as the relevant spaces.
  // and set the initial neighbours and other things..
  initSpaces(Points, Nuclei);
  clusterAncestors.clear();
  pointIndex = pointsIndex;
  for(unsigned int i=0; i < points.size(); ++i)
    initCluster(points[i]);
  cout << "\nmapPoints done " << endl;
}

vector<int> NNMapper2::pointNuclearIds()
{
  vector<int> ids( points.size() );
  for(unsigned int i=0; i < points.size(); ++i)
    ids[i] = points[i]->perimeter_id;
  return(ids);
}

vector<int> NNMapper2::pointGroupIds()
{
  vector<int> ids( points.size() );
  for(unsigned int i=0; i < points.size(); ++i)
    ids[i] = points[i]->id;
  return(ids);
}

vector<unsigned int> NNMapper2::pointIndices()
{
  return(pointIndex);
}

void NNMapper2::initSpaces(vector<threeDPoint>& Points, vector<Perimeter>& Nuclei){
  cout << "initSpaces" << endl;
  perimeters = Nuclei;
  pointSpace = new Space();
  perimeterSpace = new Space();
  points.reserve(Points.size());
  for(unsigned int i=0; i < Points.size(); ++i)
    points.push_back( pointSpace->insertPoint( Points[i].x, Points[i].y, Points[i].z ) );
  int x, y, z;
  z = 0;
  for(unsigned int i=0; i < Nuclei.size(); ++i){
    for(unsigned int j=0; j < Nuclei[i].length(); ++j){
      Nuclei[i].pos(j, x, y);
      perimeterSpace->insertPoint( x, y, z );
    }
  }
  cout << "calling setNucleus" << endl;
  setNucleus();
  cout << "set Nucleus returned setting neighbors" << endl;
  for(unsigned int i=0; i < points.size(); ++i){
    //setNeighbor(points[i]);
    setNuclearDistance(points[i]);
  }
}

void NNMapper2::initCluster(Point* point)
{
  setNeighbor(point);
  //  cout << "initCluster point id : " << point->id << "  perimeter id : " << point->perimeter_id << "  nearest distance : " << point->neighbor_sq_distance
  //     << "  nearest perimeter : " << point->nearest_perimeter_sq_distance << endl;
  if(-1 != point->id){       // point has already been defined..
    //cout << ".";
    return;
  }
  if(-1 != point->perimeter_id){  // within a nucleus. Don't bother..
    //cout << "x";
    return;
  }
  
  // point->nearetst_perimeter_id is only set if less than sq_maxD
  if(point->nearest_perimeter_id != -1 
     && 
     ( (!point->neighbor) || point->nearest_perimeter_sq_distance < point->neighbor_sq_distance)){    // nearest nuclues is nearer than nearest point
    point->perimeter_id = point->nearest_perimeter_id;
    //cout << "o";
    return;
  }
  // equally point->neighbor is only set if distance is less thand sq_maxD (otherwise it's 0, not good)
  if(!point->neighbor){   // if second is true (no neighbour), then distance is -1, so not good. 
    //cout << ":";
    point->id = 0;  // singlet
    return;
  }
  // if that true we can initiate a cluster ancestor
  int id = 1 + (int)clusterAncestors.size();
  clusterAncestors.insert(make_pair(id, point));
  point->id = id;
  multimap<int, Point*> cluster;
  vector<Point*> members;
  cluster.insert(make_pair(point->neighbor_sq_distance, point));
  members.push_back(point);
  cout << id << "  initCluster calling grow : " << point << endl;
  cout << point->x << "," << point->y << "," << point->z;
  growCluster(cluster, members);
  cout << endl;
}

// maybe need to reshift the below..
// this was supposed to be simple: keep nearest neighbor distances in a map (distance -> point)
// and then add the one with the smallest distance. The problems is that many points within the cluster
// can point to the same point (still lying outside of the cluster). Hence when a point is added to the
// cluster, any member points that still point to that need to be reassigned. (Which will lead to longer
// distances..). We don't actually need to do this for all every time, just the 

// the map cluster should only contain points that have neighbours that can be allocated
// member_points contains all the member points of the cluster and is used for assigning
// the nuclear ids when required.
void NNMapper2::growCluster(multimap<int, Point*>& cluster, vector<Point*>& member_points)
{
  multimap<int, Point*>::iterator it;
  while(cluster.size()){
    it = cluster.begin();
    if((*it).second->id != (*it).second->neighbor->id)  // the neighbor should always be specified. Otherwise we have made a mistake
      break;
    Point* p = (*it).second;
    setNeighbor( p );
    cluster.erase(it);
    if(p->neighbor)
      cluster.insert(make_pair( p->neighbor_sq_distance, p ));
  }
  if(!cluster.size())        // The cluster map only contains those members that have growable neighbours..
    return;
  // if the distance is above the max distance, simply return.
  if( (*it).first > sq_maxD )
    return;
  Point* point = (*it).second;
  Point* newPoint = point->neighbor;
  
  if(!newPoint){     // this check might be unnecessary since we check stuff above.
    cout << "unable to obtain neighbor point distance : " << point->neighbor_sq_distance << " == " << (*it).first << endl;
    return;
  }
  if(point->nearest_perimeter_id != -1 &&  point->nearest_perimeter_sq_distance < (*it).first){
    assignNuclearIds( member_points, point->nearest_perimeter_id);
    cout << "Nuclear perimeter closer than point : cluster size : " << cluster.size() << "  id : " << point->nearest_perimeter_id << endl;
    return;
  }
  if(newPoint->perimeter_id != -1){
    assignNuclearIds( member_points, newPoint->perimeter_id );
    cout << "Found point with perimeter defined cluster size : " << cluster.size() << "id : " << newPoint->perimeter_id << endl;
    return;
  }
  /////// newPoint should never have a group id defined but not a nuclear id, hence
  /////// if the logic and implementation of that logic is correct the below should
  /////// never be true.. 
  if(newPoint->id != -1){
    cerr << "NNMapper2 growCluster newPoint has group id but no perimeter_id logic or implementation error : " << newPoint->id << endl;
    exit(1);
  }
  // then simply
  newPoint->id = point->id;
  member_points.push_back(newPoint);
  cout << "  " << newPoint->x << "," << newPoint->y << "," << newPoint->z;
  //  cout << " set : " << newPoint << "=" << newPoint->id << endl;
  cluster.erase(it); 
  setNeighbor( point );  
  setNeighbor( newPoint ); // both need to be reset.. 
  //cout << "  new neighbors : " << point->neighbor << " : " << newPoint->neighbor << endl;
  if(point->neighbor)
    cluster.insert(make_pair( point->neighbor_sq_distance, point ));
  if(newPoint->neighbor)
    cluster.insert(make_pair( newPoint->neighbor_sq_distance, newPoint ));
  growCluster(cluster, member_points);
}

void NNMapper2::setNeighbor(Point* p){
  vector<Point*> npoints = pointSpace->points(p->x, p->y, p->z, maxD);
  //cout << "NNMapper2 setNeighbor points size : " << npoints.size() << endl;
  if(!npoints.size()) // shouldn't be necessary to set it.. 
    return;
  Point* closestPoint = 0;
  int sq_minD = sq_maxD * 2;  // which is higher than we allow
  for(unsigned int i=0; i < npoints.size(); ++i){
    if(npoints[i] == p)
      continue;
    int d = sq_pt_distance(p, npoints[i]);
    //    cout << d << "=" << npoints[i] << " ";
    if(d < sq_minD && (p->id == -1 || p->id != npoints[i]->id)){
      closestPoint = npoints[i];
      sq_minD = d;
    }
  }
  //cout << "  min : " << sq_minD << endl;
  if(closestPoint && sq_minD < sq_maxD){
    p->neighbor = closestPoint;
    p->neighbor_sq_distance = sq_minD;
    return;
  }
  p->neighbor = 0;
  p->neighbor_sq_distance = 0;
}

void NNMapper2::setNucleus()
{
  for(unsigned int i=0; i < perimeters.size(); ++i){
    for(unsigned int j=0; j < points.size(); ++j){
      if(perimeters[i].contains(points[j]->x, points[j]->y)){
	points[j]->perimeter_id = i;
	points[j]->nearest_perimeter_sq_distance = 0;
	points[j]->nearest_perimeter_id = i;
      }
    }
  }
}

void NNMapper2::setNuclearDistance(Point* p)
{
  if(p->perimeter_id != -1)
    return;
  vector<Point*> npoints = perimeterSpace->points(p->x, p->y, p->z, maxD);
  // find the closest point and set nearest nucleus id..
  int sq_minD = sq_maxD * 2;
  Point* nearestPerimeter = 0;
  for(unsigned int i=0; i < npoints.size(); ++i){
    int d = sq_plane_distance(p, npoints[i]);
    if(d < sq_minD){
      sq_minD = d;
      nearestPerimeter = npoints[i];
    }
  }
  if(sq_minD <= sq_maxD){
    p->nearest_perimeter_id = nearestPerimeter->id;
    p->nearest_perimeter_sq_distance = sq_minD;
  }
}

// void NNMapper2::reassignClusterIds(int old_id, int new_id, int nuclear_id)
// {
//   if(!clusterAncestors.count(old_id) || !clusterAncestors.count(new_id)){
//     cerr << "NNMapper2 reassignClusterIds, new or old ancestor unknown, will exit" << endl;
//     exit(1);
//   }
//   Point* ancestor = clusterAncestors[ old_id ];
//   assignClusterId(ancestor, new_id);
//   clusterAncestors.erase(old_id);
// }

// void NNMapper2::assignClusterId(Point* point, int id, int nuclear_id)
// {
//   point->id = id;
//   point->perimeter_id = nuclear_id;
//   for(set<Point*>::iterator it = point->daughters.begin(); it != point->daughters.end(); ++it)
//     assignClusterId((*it), id);
// }

void NNMapper2::assignNuclearIds(vector<Point*>& points, int n_id)
{
  for(vector<Point*>::iterator it=points.begin(); it != points.end(); ++it)
    (*it)->perimeter_id = n_id;
}

// void NNMapper2::assignNuclearId(Point* point, int n_id)
// {
//   point->perimeter_id = n_id;
//   for(set<Point*>::iterator it = point->daughters.begin(); it != point->daughters.end(); ++it)
//     assignNuclearId((*it), n_id);
// }

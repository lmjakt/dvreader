#include "NNMapper2.h"
#include "CellTracer.h"
#include "../../imageBuilder/OutlineTracer.h"
#include "../../imageBuilder/CellOutlines.h"
#include <iostream>
#include <stdlib.h>
#include <algorithm>

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

// use cellTracer with temporary masks to determine the cell shape..
// Note that perimeter ids have bene set as perimeters index + 1 /////
CellOutlines* NNMapper2::cellMask2D(unsigned short* mask, int xoff, int yoff, unsigned int width, unsigned int height, 
			   unsigned int max_distance, unsigned short border_increment, bool clearMask)
{
  if(clearMask)
    memset((void*)mask, 0, sizeof(unsigned short) * width * height);
  
  if(!perimeters.size())
    return(0);
  int global_width = perimeters[0].g_width();
  int global_height = perimeters[0].g_height();

  map<int, vector<Point*> > cellPoints;
  for(unsigned int i=0; i < points.size(); ++i){
    if(points[i]->perimeter_id > 0 && points[i]->id != -1)  // id != -1 excludes points within nuclei
      cellPoints[ points[i]->perimeter_id ].push_back(points[i]);
  }
  CellTracer tracer(pointSpace);
  CellOutlines* cells = new CellOutlines();
  cells->cellMask = mask;
  for(map<int, vector<Point*> >::iterator it=cellPoints.begin(); it != cellPoints.end(); ++it){
    cout << "Making map for perimeter with id : " << (*it).first << endl;
    if(((*it).first - 1) >= (int)perimeters.size())
      break;
    int mask_x, mask_y, mask_width, mask_height;
    unsigned char* cell_mask = tracer.makeCellMask(perimeters[ (*it).first - 1], (*it).second, max_distance, 
					      mask_x, mask_y, mask_width, mask_height);
    OutlineTracer outlineTracer(cell_mask, mask_x, mask_y, mask_width, mask_height, global_width, false);
    vector<int> outline = outlineTracer.traceOutline( CellTracer::border );
    cells->nuclei.push_back( perimeters[ (*it).first - 1] );
    cells->cells.push_back( Perimeter( outline, global_width, global_height ) );
    cout << "Making a new cell perimeter length is " << cells->cells.size() << " : "  << outline.size() << endl;
    cout << "With parameters : " << cells->cells.back().xmin() << " -> " << cells->cells.back().xmax() << endl;
    // and copy non_outside values..
    for(int y=0; y < mask_height; ++y){
      if((y + mask_y) >= (yoff + (int)height))
	break;
      for(int x=0; x < mask_width; ++x){
	if((x + mask_x) >= (xoff + (int)width))
	  break;
	if(cell_mask[ y * mask_width + x ] != tracer.outside)
	  mask[ (y + mask_y - yoff) * width + (x + mask_x - xoff) ] = (unsigned short)(*it).first;
	if(cell_mask[ y * mask_width + x ] & tracer.border)
	  mask[ (y + mask_y - yoff) * width + (x + mask_x - xoff) ] += border_increment;
      }
    }
    delete []cell_mask; 
  }
  return(cells);
}

// // any given position is given the identity of the closes point as long as that is not -1 or 0. 
// void NNMapper2::cellMask2D(unsigned short* mask, int xoff, int yoff, unsigned int width, unsigned int height, bool clearMask)
// {
//   if(clearMask)
//     memset((void*)mask, 0, sizeof(unsigned short) * width * height);
//   for(int y=yoff; y < (yoff + (int)height); ++y){
//     cout << "y : " << y << endl;
//     for(int x=xoff; x < (xoff + (int)width); ++x){
//       Point* f_point = pointSpace->first_point(x, y);
//       if(f_point){
// 	if(f_point->perimeter_id > 0)
// 	  mask[ (y - yoff) * width + (x - xoff) ] = f_point->perimeter_id;
// 	continue;
//       }
//       vector<Point*> npoints;
//       int radius = (maxD > 0) ? 0 : maxD;  // maxD is max distance, not maxRadius
//       while(!npoints.size() && radius <= maxD){
// 	npoints = pointSpace->plane_points(x, y, radius);
// 	radius = (radius < 2) ? radius + 1 : radius * 2;   
// 	radius = (radius > maxD && radius < 2 * maxD) ? maxD : radius;
//       }
//       if(!npoints.size())
// 	continue;
//       int minDist = sq_plane_distance(npoints[0], x, y);
//       unsigned int min_i = 0;
//       for(unsigned int i=1; i < npoints.size(); ++i){
// 	int d = sq_plane_distance(npoints[i], x, y);
// 	if(d < minDist){
// 	  minDist = d;
// 	  min_i = i;
// 	}
//       }
//       if(minDist < sq_maxD && npoints[min_i]->perimeter_id > 0)
// 	mask[ (y - yoff) * width + (x - xoff) ] = npoints[min_i]->perimeter_id;
//     }
//   }
// }

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
      Point* p = perimeterSpace->insertPoint( x, y, z );
      p->id = i+1;
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
  if(-1 != point->id){       // point has already been defined..
    return;
  }
  if(-1 != point->perimeter_id){  // within a nucleus. Don't bother..
    return;
  }
  
  // point->nearetst_perimeter_id is only set if less than sq_maxD
  if(point->nearest_perimeter_id != -1 
     && 
     ( (!point->neighbor) || point->nearest_perimeter_sq_distance < point->neighbor_sq_distance)){    // nearest nuclues is nearer than nearest point
    point->perimeter_id = point->nearest_perimeter_id;
    return;
  }
  // equally point->neighbor is only set if distance is less thand sq_maxD (otherwise it's 0, not good)
  if(!point->neighbor){   // if second is true (no neighbour), then distance is -1, so not good. 
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
  growCluster(cluster, members);
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
    return;
  }
  if(point->nearest_perimeter_id != -1 &&  point->nearest_perimeter_sq_distance < (*it).first){
    assignNuclearIds( member_points, point->nearest_perimeter_id);
    return;
  }
  // then newPoint will be used, so let's add to the daughters..
  point->daughters.insert(newPoint);
  if(newPoint->perimeter_id != -1){
    assignNuclearIds( member_points, newPoint->perimeter_id );
    return;
  }
  /////// newPoint should never have a group id defined but not a nuclear id, hence
  /////// if the logic and implementation of that logic is correct the below should
  /////// never be true.. 
  if(newPoint->id != -1){
    cerr << "NNMapper2 growCluster newPoint has group id but no perimeter_id logic or implementation error : " 
	 << newPoint->id << endl;
    exit(1);
  }
  // then simply
  newPoint->id = point->id;
  member_points.push_back(newPoint);

  cluster.erase(it); 
  setNeighbor( point );  
  setNeighbor( newPoint ); // both need to be reset.. 
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
  int nuclear_point_count = 0;
  for(unsigned int i=0; i < perimeters.size(); ++i){
    for(unsigned int j=0; j < points.size(); ++j){
      if(perimeters[i].contains(points[j]->x, points[j]->y)){
	points[j]->perimeter_id = i+1;
	points[j]->nearest_perimeter_sq_distance = 0;
	points[j]->nearest_perimeter_id = i;
	++nuclear_point_count;
      }
    }
  }
  cout << "\nNuclear Point Count : " << nuclear_point_count << endl;
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

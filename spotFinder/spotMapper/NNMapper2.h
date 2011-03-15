#ifndef NNMAPPER2_H
#define NNMAPPER2_H

#include "../../dataStructs.h"
#include "../../abstract/space.h"
#include "../perimeter.h"
#include <vector>
#include <map>

struct CellOutlines;

class NNMapper2 
{
 public:
  NNMapper2(int maxDistance);
  ~NNMapper2();

  void setMaxDistance(int maxDistance);
  //  void mapPoints(std::vector<threeDPoint>& Points, std::vector<Perimeter>& Nuclei);
  void mapPoints(std::vector<threeDPoint>& Points, std::vector<unsigned int> pointsIndex, std::vector<Perimeter>& Nuclei);
  CellOutlines* cellMask2D(unsigned short* mask, int xoff, int yoff, unsigned int width, unsigned int height, 
		  unsigned int max_distance, unsigned short border_increment, bool clearMask=true);
  std::vector<int> pointNuclearIds();
  std::vector<int> pointGroupIds();
  std::vector<unsigned int> pointIndices();

 private:
  int sq_maxD;  // the squared max distance between two points in a cluster (or next to a nucleus);
  int maxD;
  std::vector<Point*> points;  // these keep the same order as the input threeDPoints
  std::vector<unsigned int> pointIndex;  // a set of indices set by the user.. 
  std::vector<Perimeter> perimeters; // to avoid using the perimeter space to check overlap. 
  Space* pointSpace;
  Space* perimeterSpace;  // actually two-dimensional, but, what can we do.
  std::map<int, Point*> clusterAncestors;  // The original ancestor for cluster (use as a gateway to original clusters).

  void initSpaces(std::vector<threeDPoint>& Points, std::vector<Perimeter>& Nuclei);

  void initCluster(Point* point); // uses clusterAncestors to determine the cluster ID (clusterAncestors.size()). 
  void growCluster(std::multimap<int, Point*>& cluster, std::vector<Point*>& member_points);  // int = nearest sq distance, of cluster members

  void setNeighbor(Point* p);  // finds the nearest neighbour
  void setNucleus(); // looks for overlaps. Is potentially slow.. as each nucleus is checked against each point.
  void setNuclearDistance(Point* p);   // finds and sets the nearest nucleus.
  //void reassignClusterIds(int old_id, int new_id, int nuclear_id=-1);
  //void assignClusterId(Point* point, int id, int nuclear_id=-1); // recursive uses parent -> daughter to reassign.. 
  void assignNuclearIds(std::vector<Point*>& points, int n_id);
  //void assignNuclearId(Point* point, int n_id);  // recursive


  int sq_pt_distance(Point* a, Point* b){
    return( (a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y) + (a->z - b->z) * (a->z - b->z) );
  }
  int sq_plane_distance(Point* a, Point* b){
    return( (a->x - b->x) * (a->x - b->x) + (a->y - b->y) * (a->y - b->y));
  }
  int sq_plane_distance(Point* a, int x, int y){
    return( (a->x - x) * (a->x - x) + (a->y - y) * (a->y - y) );
  }
  

};

#endif

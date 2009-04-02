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

#ifndef NEARESTNEIGHBORMAPPER_H
#define NEARESTNEIGHBORMAPPER_H

#include <QObject>
#include "../../abstract/space.h"
#include "../../dataStructs.h"
#include "../perimeter.h"

// a struct that contains a number of points all linked together in some way.
// and some method..


class NeighborCluster 
{
    int id;             // must not be negative. negative indicates some sort of error...
    Point* seed;        // this is our entry point. (a seed has only daughters, but if added to another cluster can gain parents).
    void setIds(int i, Point* p);  // two recursive functions to fill thee ids
    void rejuvenateParents(Point* newParent, Point* p);  // turns parents into daughters.. 
    void updateLimits(Point* np);
    void getPoints(std::vector<Point*>& pts, Point* p);  // recurse through and do things to the pts vector
    void growTwoDMap(Point* p, std::vector<twoDPoint>& pts);

    // to add a point, first find the shortest allowable distance within the set, and add that point..
    
    // np (new point) is the new point to be added to the cluster
    // mp (member point) is the point to which np should be added
    // minDistance is the miminum distance which determines how we handle this
    // points are the points that we can add to the cluster..
    // p  is the current member point that we are checking. If d < minDistance, then np gets set to p.. 

    void checkDistances(Point*& np, Point*& mp, int& minDistance, std::vector<Point*>& points, Point* p);  
    void checkNuclearDistances(Point* p, int& minDistance, std::vector<Perimeter>& prs, int& nid);

    int sqed(Point* a, Point* b){
	return(((a->x - b->x)*(a->x - b->x) + (a->y - b->y)*(a->y - b->y) + (a->z - b->z)*(a->z - b->z)) );
    }

    int xmin, xmax, ymin, ymax, zmin, zmax;   // so we know how big a spcae we need to use.. 
    int size;

    public :
	NeighborCluster(int i, Point* s){
	id = i;
	seed = s;
	seed->id = id;
	xmin = xmax = seed->x;
	ymin = ymax = seed->y;
	zmin = zmax = seed->z;
	size = 1;
    }
    void limits(int& xn, int& xx, int &yn, int& yx, int& zn, int& zx);  // sets to min x, max x, etc.. 
    NeighborCluster(){
	size = 0;
	seed = 0;
    }
    void incrementSize(int s){
	size += s;
    }
    int clusterSize(){
	return(size);
    }
    std::vector<twoDPoint> makeTwoDMap();
    ~NeighborCluster(){}
    // adds the closest point from the given set of points (as long as this is not circular).
    // if the nearest point has been assigned a cluster id already, then all members of this cluster will 
    // be assigned to be have ids of the other cluster. 
    // this can be checked by the user by comparing the clusterId with the returned point id. If these are different
    // then the current NeighbourCluster can easily be switched off. 
    Point* addPoint(std::vector<Point*>& points, std::vector<Perimeter>& prs, int& nid);  
    std::vector<Point*> allPoints();
    void updateLimits(NeighborCluster& nc);
    void setId(int i);  // iterate across the points and set the id
    int clusterId(){
	return(id);
    }
    int nucleusId;      // this gets set if the point overlaps with a nucleus. 
};

class NearestNeighborMapper : public QObject
{
    Q_OBJECT
	public :
	NearestNeighborMapper(int m){
	margin = m;
	space = 0;
    }
    ~NearestNeighborMapper();
    std::vector<int> mapPoints(std::vector<threeDPoint>& points, std::vector<Perimeter>& nuclei);
    std::vector<twoDPoint> mapOnePoint(threeDPoint& point, std::vector<threeDPoint>& points, std::vector<Perimeter>& nuclei);
//    std::vector<Point*> findPerimeter(std::vector<Point*>& points, int maxD);

    public slots :
	void setMargin(int m){
	margin = m;
    }

    private :
	int margin;   // the distance away from the different things that we need to use for calculations.
    Space* space;
    std::map<Point*, int> pointIndex;   // these are set up by the space
    std::vector<Point*> pointArray;
    void initSpace(std::vector<threeDPoint>& points);
    void growCluster(NeighborCluster& cluster, std::map<int, NeighborCluster>& clusters, std::vector<Perimeter>& nuclei);
    int overlapsNucleus(std::vector<Perimeter>& nuclei, Point* p);
    void setLimits(NeighborCluster& cluster, int& xb, int& xe, int& yb, int& ye, int& zb, int& ze);
//    double angleFromXaxis(Point* o, Point* p); // clockwise angle -- o is the origin point, p is the point on the circle  
//    Point* findNextPerimeterPoint(std::vector<Point*>& points, Point* cp, Point* pp, int maxD);
//    double angleBetweenPoints(Point* pp, Point* cp, Point* np);
};

#endif

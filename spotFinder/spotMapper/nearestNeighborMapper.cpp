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

#include "nearestNeighborMapper.h"
#include <math.h>
#include <iostream>

using namespace std;

void NeighborCluster::setId(int i)
{
    id = i;
// does not change ids of the points.
}

void NeighborCluster::setIds(int i, Point* p)
{
    p->id = i;
    for(set<Point*>::iterator it = p->daughters.begin(); it != p->daughters.end(); it++)
	setIds(i, (*it));
}


// Damn, I should have written the below function as a member function of point. Doing so
// would have made it much, much easier to grep it, and it would look much nicer. However,
// I had the stupid idea that I should keep the basic simple Point struct as simple as possible
// to keep down memory use. Not sure if that was a good idea now.
void NeighborCluster::rejuvenateParents(Point* newParent, Point* p)
{
    if(p->parent)
	rejuvenateParents(p, p->parent);

    if(p->parent)
	p->daughters.insert(p->parent);
    p->daughters.erase(newParent);
//    p->daughters.erase(p->parent);
    p->parent = newParent;
    newParent->daughters.insert(p);
}

void NeighborCluster::updateLimits(Point* np)
{
    xmin = xmin < np->x ? xmin : np->x;
    xmax = xmax > np->x ? xmax : np->x;
    ymin = ymin < np->y ? ymin : np->y;
    ymax = ymax > np->y ? ymax : np->y;
    zmin = zmin < np->z ? zmin : np->z;
    zmax = zmax > np->z ? zmax : np->z;
}

void NeighborCluster::updateLimits(NeighborCluster& nc)
{
    xmin = xmin < nc.xmin ? xmin : nc.xmin;
    xmax = xmax > nc.xmax ? xmax : nc.xmax;
    ymin = ymin < nc.ymin ? ymin : nc.ymin;
    ymax = ymax > nc.ymax ? ymax : nc.ymax;
    zmin = zmin < nc.zmin ? zmin : nc.zmin;
    zmax = zmax > nc.zmax ? zmax : nc.zmax;
}

// Find the nearest point.
void NeighborCluster::checkDistances(Point*& np, Point*& mp, int& minDistance, std::vector<Point*>& points, Point* p)
{
    for(uint i=0; i < points.size(); ++i){
	if(points[i]->id == p->id)             // that would be a circular join and we do not allow that.. 
	    continue;
	int d = sqed(p, points[i]);
	if(d < minDistance){
	    minDistance = d;
	    np = points[i];
	    mp = p;
	}
    }
    for(set<Point*>::iterator it = p->daughters.begin(); it != p->daughters.end(); it++)
	checkDistances(np, mp, minDistance, points, (*it));
}

void NeighborCluster::checkNuclearDistances(Point* p, int& minDistance, vector<Perimeter>& prs, int& nid)
{
    for(uint i=0; i < prs.size(); ++i){
	int d = prs[i].minSqDistance(minDistance, p->x, p->y);
//	cout << "   " << d;
	if(d <= minDistance){
	    minDistance = d;
	    nid = i;
	}
    }
//    cout << endl;
    for(set<Point*>::iterator it = p->daughters.begin(); it != p->daughters.end(); it++)
	checkNuclearDistances((*it), minDistance, prs, nid);
    
}

vector<twoDPoint> NeighborCluster::makeTwoDMap()
{
    vector<twoDPoint> pts;
    growTwoDMap(seed, pts);
    return(pts);
}

void NeighborCluster::growTwoDMap(Point* p, vector<twoDPoint>& pts)
{
    for(set<Point*>::iterator it = p->daughters.begin(); it != p->daughters.end(); it++){
	pts.push_back(twoDPoint(p->x, p->y));
	pts.push_back(twoDPoint((*it)->x, (*it)->y));
	growTwoDMap((*it), pts);
    }
}

Point* NeighborCluster::addPoint(vector<Point*>& points, vector<Perimeter>& prs, int& nid)
{
    nid = -1;
    Point* np = 0;
    Point* mp = 0;   // these are the new point and the member point..
    int minDistance = 10000000;  // ugly hack.. but ok for now, as other options also ugly
    // see if we can find a closer point:
    // this will end up checking the seed point two times, but that may not make much of a difference..
    checkDistances(np, mp, minDistance, points, seed);

    // if we cannot find any other points, then we return the null pointer
    if(!np)
	return(np);

    // check if there is a nucleus within the minium distance. If there is we don't want to add this one
    // to the thing, but instead assign the nucleus id of this cluster..

    // all nuclei have to be checked by all the points. Otherwise we cannot guarantee the appropriate behaviour
    int oldD = minDistance;

//    checkNuclearDistances(seed, minDistance, prs, nid);

//    cout << "NeighborCluster::addPoint checked nuclear distances with max : " << oldD;
//    cout << "  returned with minDistance : " << minDistance << "   nid : " << nid << endl;
    if(nid != -1)
	return(0);
    

    // if the id of np is equal to -1, then we set the id of np to id and then add it to our group
    if(np->id == -1){
	// then seperately check the np since this one 

//	checkNuclearDistances(np, minDistance, prs, nid);

	np->id = id;
	mp->daughters.insert(np);
	np->parent = mp;
	updateLimits(np);
	++size;
	return(np);
    }
    // if we are here, then np has some group id. Since that group precedes us in the ranking, we'll add all our points to 
    // that group, by changing the values of the linking pointers..   
    // this means that we have to convert all of the parents of mp to daughters of mp, which ends up setting making the seed pointer
    // leading to nowhere. This may be tricky.
    rejuvenateParents(np, mp);
    setIds(np->id, mp);    
    return(np);   // at this point this clusterGroup should be discarded as it no longer has any value. However, that is for the owner to decide
}

void NeighborCluster::limits(int& xn, int& xx, int& yn, int& yx, int& zn, int& zx)
{
    xn = xmin;
    xx = xmax;
    yn = ymin;
    yx = ymax;
    zn = zmin;
    zx = zmax;
}


vector<Point*> NeighborCluster::allPoints()
{
    vector<Point*> pts;
    getPoints(pts, seed);
    return(pts);
}    

void NeighborCluster::getPoints(vector<Point*>& pts, Point* p)
{
    pts.push_back(p);
    for(set<Point*>::iterator it = p->daughters.begin(); it != p->daughters.end(); it++)
	getPoints(pts, *it);
}

NearestNeighborMapper::~NearestNeighborMapper()
{
    if(space)
	delete space;
}

vector<int> NearestNeighborMapper::mapPoints(vector<threeDPoint>& points, vector<Perimeter>& nuclei)
{
    // First we need to set up a space with all the points in it.
    initSpace(points);
    
    // for each point that is not assigned an identity start assigning identities, until we've gone through all of the points
    int g_count = 0;
    int mergedCount = 0;
    map<int, NeighborCluster> clusters;
    map<Point*, int> nuclearPoints;
    for(vector<Point*>::iterator it = pointArray.begin(); it != pointArray.end(); it++){
	cout << "***" << endl;
	int nid = overlapsNucleus(nuclei, *it);
	if(nid != -1){
	    nuclearPoints.insert(make_pair(*it, nid));
	    continue;
	}
	cout << "---" << endl;
	if((*it)->id != -1)
	    continue;
	++g_count;
	cout << "making cluster # " << g_count << endl;
	NeighborCluster cluster(g_count, *it);
	growCluster(cluster, clusters, nuclei);
	// if the cluster id is not -1 then insert it into the map, otherwise leave it
	if(cluster.clusterId() != -1){
	    clusters.insert(make_pair(cluster.clusterId(), cluster));
	}else{
	    mergedCount++;
	}
	cout << "after growing the cluster the id is : " << cluster.clusterId() << "  and nuclear id is : " << cluster.nucleusId << endl;

    }
    cout << "alright man, we've now gone through all of the pointes " << endl;
    // This puts all of the points into clusters except for those that overlap with nuclei. 
    // These now will need to be put into clusters as well. We should try to do this, using the same 
    // functions below, -- and end up just joining them on to to the rest of the neighborhoods.. 
    
    // now we need to make a vector of the nuclearIds, and then fill it from the clusters and the others..
    vector<int> nuclearIds(points.size(), -1);
    // go through the nuclear Points first..
    for(map<Point*, int>::iterator it=nuclearPoints.begin(); it != nuclearPoints.end(); it++){
	nuclearIds[pointIndex[it->first]] = it->second;
    }
    cout << "after going through the nuclearIds map " << endl;
    // and then go through the groups
    int pointCounter = 0;
    for(map<int, NeighborCluster>::iterator it = clusters.begin(); it != clusters.end(); it++){
	cout << "--- do we get here ---" << endl;
	vector<Point*> pts = it->second.allPoints();
	cout << "got " << pts.size() << "  points from cluster with size : " << it->second.clusterSize() <<  " and id : " << it->first << "  and with a nucleus id of " << it->second.nucleusId << endl;
	pointCounter += pts.size();
	for(vector<Point*>::iterator pt=pts.begin(); pt != pts.end(); pt++)
//	    nuclearIds[pointIndex[*pt]] = it->second.clusterId();
	    nuclearIds[pointIndex[*pt]] = it->second.nucleusId;
    }
    cout << "Total number of clusters initiated : " << g_count << "  total merged : " << mergedCount << "  giving a total of : " << clusters.size() << "  clusters " << endl;
    cout << "Total of " << pointCounter << "  points were obtained from the thingy" << endl;
    return(nuclearIds);
}

vector<twoDPoint> NearestNeighborMapper::mapOnePoint(threeDPoint& point, vector<threeDPoint>& points, vector<Perimeter>& nuclei){
    initSpace(points);
    Point* p = space->insertPoint(point.x, point.y, point.z);  // it is probably already there, but .. 
    NeighborCluster cluster(1, p);
    int nid = overlapsNucleus(nuclei, p);
    vector<twoDPoint> clusterMap;
    if(nid != -1){
	cout << "NearestNeighborMapper::mapOnePoint, point overlaps nucleus : " << nid << endl;
	return(clusterMap);
    }
    map<int, NeighborCluster> clusters;  // I don't actually need this but the other function takes
    growCluster(cluster, clusters, nuclei);
    cout << "Mapped on point to a cluster with nucelar id  " << cluster.nucleusId << endl;
    clusterMap = cluster.makeTwoDMap();
    return(clusterMap);
}
 
void  NearestNeighborMapper::initSpace(vector<threeDPoint>& points)
{
    if(space)
	delete space;
    space = new Space();
    pointIndex.clear();
    for(uint i=0; i < points.size(); ++i){
	Point* p = space->insertPoint(points[i].x, points[i].y, points[i].z);
	pointIndex.insert(make_pair(p, i));
	pointArray.push_back(p);
    }
}

void NearestNeighborMapper::setLimits(NeighborCluster& cluster, int& xb, int& xe, int& yb, int& ye, int& zb, int& ze)
{
    int xmin, xmax, ymin, ymax, zmin, zmax;
    cluster.limits(xmin, xmax, ymin, ymax, zmin, zmax);
    xb = (xmin - margin) > 0 ? (xmin - margin) : 0;
    yb = (ymin - margin) > 0 ? (ymin - margin) : 0;
    zb = (zmin - margin) > 0 ? (zmin - margin) : 0;
    xe = xmax + margin;
    ye = ymax + margin;
    ze = zmax + margin;
}

void NearestNeighborMapper::growCluster(NeighborCluster& cluster, map<int, NeighborCluster>& clusters, vector<Perimeter>& nuclei)
{
    int xb, xe, yb, ye, zb, ze;
    setLimits(cluster, xb, xe, yb, ye, zb, ze);
    vector<Point*> pts = space->points(xb, xe, yb, ye, zb, ze);
    cout << "Growcluster initial point request got : " << pts.size() << endl;
    cout << "From the coordinates : " << xb << "," << xe << " : " << yb << "," << ye << " : " << zb << "," << ze << endl;
    Point* point = 0;
    int nid;
    if(!pts.size()){
	cerr << "growCluster received no points. This shouldn't be possible ?" << endl;
	return;
    }
    while(true){
	point = cluster.addPoint(pts, nuclei, nid);  // i.e. doesn't equal to 0
//	if(!point->id)                  // no reasonable point found. But, maybe we should expand the margin ?
	if(!point && nid == -1){
	    cout << "growCluster no suitable point found :: ??? " << endl;
	    break;
	}
	// check the value of nid..
	if(nid != -1){
	    cluster.nucleusId = nid;
	    cout << "Found a proximal nucleus .." << endl;
	    break;
	}
	if(point->id != cluster.clusterId()){    // then we need to 
	    clusters[point->id].updateLimits(cluster);
	    clusters[point->id].incrementSize(cluster.clusterSize());
	    cluster.setId(-1);           // to indicate that we don't like this cluster anymore..
	    cout << "merging with another cluster and setting the id of this cluster to -1" << endl;
	    break;
	}

	nid = overlapsNucleus(nuclei, point);
	if(nid != -1){
	    cluster.nucleusId = nid;
	    cout << "Found a nucleus with id : " << nid << endl;
	    break;
	}
	// At this point we need to work out whether or not we need to refresh pts. This depends on whether the clusterGroup
	// has changed markedly or not.
	int xb2, xe2, yb2, ye2, zb2, ze2;
	setLimits(cluster, xb2, xe2, yb2, ye2, zb2, ze2);
	int maxD = margin / 2;
	if(xb - xb2 > maxD ||
	   xe2 - xe > maxD ||
	   yb - yb2 > maxD ||
	   ye2 - ye > maxD ||
	   zb - zb2 > maxD ||
	   ze2 - ze > maxD){
	    xb = xb2; xe = xe2;
	    yb = yb2; ye = ye2;
	    zb = zb2; ze = ze2;
	    pts = space->points(xb, xe, yb, ye, zb, ze);
	    cout << "refreshing points from space got " << pts.size() << "  points "  << endl;
	}
    }
    // 
}

int NearestNeighborMapper::overlapsNucleus(vector<Perimeter>& nuclei, Point* p)
{
    for(uint i=0; i < nuclei.size(); i++){
	if(nuclei[i].contains(p->x, p->y))
	    return((int)i);
    }
    return(-1);
}


// Finds a perimeter for the set of points by looking for the maximum (clockwise) from the preceding point
// Uses maxD to constrain the algorithm to allow for concave shapes (otherwise we would end up with much hassle).

// vector<Point*> NearestNeighborMapper::findPerimeter(vector<Point*>& points, int maxD)
// {
//     // First find the point with the smallest y value, and make this the starting point.
//     // For the first round, assume that the previous point lies directly to the right of the starting point.
//     // Then find the next point with the closest angle..
//     vector<Point*> perimeter;
//     if(!points.size())
// 	return(perimeter);
//     Point* sp = points[0];
//     for(uint i=1; i < points.size(); i++){
// 	if(sp->y > points[i]->y)
// 	    sp = points[i];
//     }
//     perimeter.push_back(sp);
//     Point psp(sp->x + 1, sp->y, sp->z);  // the pseudopoint
//     Point* pp = &psp;
//     Point* cp = sp;
//     Point* np = 0;

//     while(np != sp){
// 	int max_d = maxD;
// 	while( !( np = findNextPerimeterPoint(points, cp, pp, max_d) ) )
// 	    max_d += maxD;
// 	perimeter.push_back(np);
// 	pp = cp;
// 	cp = np;
//     }
//     return(perimeter);
// }

// // cp is the current point. pp is the previous point.. 
// Point* NearestNeighborMapper::findNextPerimeterPoint(vector<Point*>& points, Point* cp, Point* pp, int maxD)
// {
//     Point* np = 0;   // the next point.. 
//     maxD *= maxD;    // then we don't have to call sqrt
//     double minAngle = 4 * M_PI;   // all angles will be less than this..
//     for(uint i=0; i < points.size(); i++){
// 	if(points[i] == cp || points[i] == pp)
// 	    continue;
// 	if( ((points[i]->x - cp->x) * (points[i]->x - cp->x) + (points[i]->y - cp->y) * (points[i]->y - cp->y) ) > maxD)
// 	    continue;
// 	double angle = angleBetweenPoints(pp, cp, points[i]);
// 	if(angle < minAngle){
// 	    minAngle = angle;
// 	    np = points[i];
// 	}
//     }
//     return(np);
// }

// double NearestNeighborMapper::angleFromXaxis(Point* o, Point* p)
// {
//     // If point o is considered to lie at the origin of a circle, and the vector
//     // from o to p (p - o) is scaled to have a magnitude of 1 then :
//     // 
//     // sin P = -dy    (-y since y is for the anti - clockwise angle.. )
//     // cos P = dx
//     // if both are calculated it is possible to determine the angle and the directionality of it
    
//     double dx = (double)(p->x - o->x);
//     double dy = (double)(o->y - p->y);  // i.e. - dy .. (we could also change this by 
//     // then scale dx and dy accordingly. Do this the slow way to avoid making mistakes
//     double l = sqrt( (dx * dx) + (dy * dy) );
//     dx = dx / l;
//     dy = dy / l;   // now the length of dx, dy is 1.

//     // if dy is positive return acos(dx)
//     // otherwise return M_PI + (M_PI - acos(dx)) or 2 * M_PI - acos(dx);
//     if(dy >= 0)
// 	return(acos(dx));
//     return(2 * M_PI - acos(dx));
// }

// // pp previous point, cp current point, np next point 
// double NearestNeighborMapper::angleBetweenPoints(Point* pp, Point* cp, Point* np)
// {
//     // Find the clockwise angle made from the vector pp, cp, np
//     // i.e. imagine a circle with the origin at cp, and lines np and pp joining this circle.
    
//     // Find the angle from vector pp -> cp and np -> cp.
//     // Follow this method:
//     // First scale both vectors to have a magnitude of 1. (This makes sins and cosines kind of easy).
//     // Rotate both vectors anti-clockwise such that cp -> pp lies along the x-axis. 
//     // That is rotate both vectors by the clockwise angle from the x-axis to the vector cp -> pp.
//     // Then simply calculate the angle by the x and y coordinates ..
//     double pp_x = (double)(pp->x - cp->x);
//     double pp_y = (double)(pp->y - cp->y);
//     pp_x /= sqrt( (pp_x * pp_x) + (pp_y * pp_y) );   
//     pp_y /= sqrt( (pp_x * pp_x) + (pp_y * pp_y) );   // scales the vectors to unit length
    
//     double np_x = (double)(np->x - cp->x);
//     double np_y = (double)(np->y - cp->y);
//     np_x /= sqrt( (np_x * np_x) + (np_y * np_y) );
//     np_y /= sqrt( (np_x * np_x) + (np_y * np_y) );

//     // In this case, if we call the clockwise angle from the x-axis to the vector cp -> pp as a
//     // we can say that :
//     // cos(a) = pp_x
//     // sin(a) = -pp_y     (negative since the angle is clockwise rather than anticlockwise).
    
//     // To rotate anti - clockwise the vector cp -> np by the angle a we use the standard rotation matrix :
//     // |x'| = |cos(a)  -sin(a)| |x|
//     // |y'|   |sin(a)   cos(a)| |y|
//     // 
//     // which is a bit difficult to read but which ends up being :
//     // x' = x * cos(a) - y * sin(a)
//     // y' = x * sin(a) + y * cos(a)
//     //
//     // However, since for this special occasion :
//     // cos(a) = pp_x
//     // sin(a) = -pp_y;
//     // and x = np_x and y = np_y
//     // x' = np_x * pp_x - (np_y * -pp_y)
//     // y' = np_x * -pp_y + (np_y * pp_x)
//     double x = np_x * pp_x - (np_y * -pp_y);
//     double y = np_x * -pp_y + (np_y * pp_x);
//     // and then simply : 
//     if(y >= 0)
// 	return(acos(x));
//     return(2 * M_PI - acos(x));
// }

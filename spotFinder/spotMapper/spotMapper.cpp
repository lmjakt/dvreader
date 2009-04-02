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

#include "spotMapper.h"
#include <iostream>

using namespace std;

SpotMapper::SpotMapper(int ms, double ss, double s, double l, double rs)
{
	maxStep = ms;
	stepSize = ss;
	sigma = s;
	repSigma = rs;
	/// we use the following equation to calculate the magnitude of the influence
	/// inf = 1 - (d^2 / (sigma + d^2) )
	/// where inf = degree of influence
	///         d = euclidean distance
	///     sigma = a spreading factor (similar to std in a normal distribution ?)
	///
	/// since for the equation  (d^2 / (sigma + d^2) ) 
	/// d = sqrt( sigma * y / (1 - y) )
	// we can thus use this equation to set a minimum distance. In this case we'll set a minimum influence of 0.01
	// or y = 0.99 in the above equation. We will then use this as a maximum distance to avoid having to perform 
	// lots of additional calculations..
	limit = 1.0 - l;
	maxD =  (sigma * limit) / (1 - limit);
}

void SpotMapper::setMaxD(){
    	maxD =  (sigma * limit) / (1 - limit);
}

vector<int> SpotMapper::walkPoints(vector<threeDPoint>& points, vector<Perimeter>& nuclei)
{
    vector<int> nuclearIds(points.size(), -1);
    // and then simply..
    vector<threeDPoint> path;
    path.reserve(maxStep);
    int success = 0;
    for(uint i=0; i < points.size(); ++i){
	if(!(i % (points.size() / 100))){
	    cout << i << "\t: " << i / (points.size() / 100) << " %" << endl;
	}
	path.resize(0);  // make sure it is empty.. 
	if(walkOnePoint(points[i], points, nuclearIds[i], path, nuclei))
	    ++success;
	points[i].id = nuclearIds[i];   // a little bit superfluous. 
    }
    cout << "walk Points assigned ids to : " << success << "  out of a total of : " << points.size() << "  points " << endl;
    return(nuclearIds);
}

bool SpotMapper::walkOnePoint(threeDPoint& point, vector<threeDPoint>& points, int& nucleus, vector<threeDPoint>& path, vector<Perimeter>& nuclei)
{
    // First check if point is present in any of the nuclei. If this, is the case then return it.
    nucleus = -1;              // the default
    path.push_back(point);     // the path always contains the starting point.
    if(checkNuclei(point, nuclei, nucleus))
	return(true);
    
//    cout << "walk one point maxD is " << maxD << "  sigma " << sigma << "  stepSize  " << stepSize << endl;

    for(int step=0; step < maxStep; ++step){
	nextStep(path.back(), points, path, nuclei);
	if(checkNuclei(path.back(), nuclei, nucleus))
	    return(true);
	
    }
    return(false);
}

void SpotMapper::nextStep(threeDPoint& point, std::vector<threeDPoint>& points, std::vector<threeDPoint>& path, std::vector<Perimeter>& nuclei)
{
    double dx, dy, dz;
    dx = dy = dz = 0.0;
    for(uint i=0; i < points.size(); ++i){
	if(point == points[i])
	    continue;
	calculateVector(point, points[i], dx, dy, dz, 1.0, sigma);
    }
    for(uint i=0; i < path.size(); i++){
	if(point == points[i])
	    continue;
	calculateVector(point, path[i], dx, dy, dz, -1.0, repSigma);
    }
    // I now have a vector pointing to somewhere, and I want to scale it such that it gets size stepSize .. 
    // amplitude of the vector is :  sqrt( (dx * dx) + (dy * dy) + (dz * dz) ) 
    // so the multiplier mp is :
    double mp = stepSize / sqrt( (dx * dx) + (dy * dy) + (dz * dz) );
//    cout << "\t dx : dy : dz " << dx << " : " << dy << " : " << dz  << "  mp : " << mp ;
    
    // then the new points will be :
    double x = point.xd + mp * dx;
    double y = point.yd + mp * dy;
    double z = point.zd + mp * dz;
//    cout << "  new point : " << x << ", " << y << ", " << z << endl;
    path.push_back(threeDPoint(x, y, z));
}

void SpotMapper::calculateVector(threeDPoint& a, threeDPoint& b, double& dx, double& dy, double& dz, double m, double s){
    double d = a.sq_ed(b);  // this does not call square root and so saves lots of time.. 
    if(d > maxD)
	return;
    double f = 1 - (d) / (s + d);
//    cout << "\t\t d " << d << "  f : " << f << endl; 
    f *= m;   // a should be 1 or -1
    dx += f * (b.xd - a.xd);
    dy += f * (b.yd - a.yd);
    dz += f * (b.zd - a.zd);
}
    
bool SpotMapper::checkNuclei(threeDPoint& point, vector<Perimeter>& nuclei, int& nucleus)
{
    nucleus = -1;
    for(uint i=0; i < nuclei.size(); ++i){
	if(nuclei[i].contains(point.x, point.y)){
	    nucleus = (int)i;
	    return(true);
	}
    }
    return(false);
}

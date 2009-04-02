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

#ifndef CELL_H
#define CELL_H

#include <vector>
#include <map>
#include "perimeter.h"
#include "../abstract/space.h"
#include <math.h>

struct HybPoint
{
    int x, y, z;
    int waveIndex;
    float waveLength;
    double density;
    double wlDensity;
    
    HybPoint(){
	x = y = z = 0;
	waveIndex = -1;
	waveLength = 0;
	density = wlDensity = 0;
    }
    HybPoint(int X, int Y, int Z, int wi, float wl){
	x = X; y = Y; z = Z;
	waveIndex = wi;
	waveLength = wl;
    }
    double distance(HybPoint& p){
	return(sqrt(double( (x - p.x) * (x - p.x) + (y - p.y) * (y - p.y) + (z - p.z) * (z - p.z))));
    }
};

class Cell
{
    friend class SpotMapperWindow;   // I don't have enought time at the moment.. 
    Perimeter nucleus;  // the problem is that one PerimeterSet can contain more than one nucleus.
    std::vector<Point> cellPerimeter;
    std::map<int, std::vector<HybPoint> > hybPoints;
    std::map<int, float> waveLengths;
    std::map<int, double> meanPointDensities;
    double meanPointDensity;
    void determinePointDensities();

    public :
	Cell(){
	meanPointDensity = -1;
    }
    Cell(Perimeter n, std::vector<Point> cp){
	nucleus = n;
	cellPerimeter = cp;
	meanPointDensity = -1;
    }
    void addHybPoint(HybPoint& p){
	hybPoints[p.waveIndex].push_back(p);
	waveLengths[p.waveIndex] = p.waveLength;
	meanPointDensity = -1;
    }
    double MeanPointDensity(){
	if(meanPointDensity < 0)
	    determinePointDensities();
	return(meanPointDensity);
    }
    double MeanPointDensity(int wi){
	if(!hybPoints.count(wi))
	    return(-1.0);
	if(meanPointDensity < 0)
	    determinePointDensities();
	return(meanPointDensities[wi]);
    }
    std::map<int, float> waves(){
	return(waveLengths);
    }
    std::vector<HybPoint> points(int wi){
	if(hybPoints.count(wi))
	    return(hybPoints[wi]);
	std::vector<HybPoint> pt;
	return(pt);
    }
    int pointNo(int wi){
	if(hybPoints.count(wi))
	    return((int)hybPoints[wi].size());
	return(-1);
    }
};


#endif

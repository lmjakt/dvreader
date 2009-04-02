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

#include "cell.h"
#include <iostream>
#include <math.h>

using namespace std;

// at some point we should make the variables user managed, but now I'm tired.. 
void Cell::determinePointDensities()
{
    double b = 1.4;
    double sigma = 100.0;
    meanPointDensity = 0.0;
    map<int, vector<HybPoint> >::iterator it;
    int totalCount = 0;
    for(it = hybPoints.begin(); it != hybPoints.end(); it++){
	totalCount += (*it).second.size();
	meanPointDensities[(*it).first] = 0.0;
	vector<HybPoint>& points = (*it).second;
	for(uint i=0; i < points.size(); i++){
	    points[i].density = 0;
	    points[i].wlDensity = 0;
	    // first the wlDensity which is easier to handle
	    for(uint j=0; j < points.size(); ++j){
		double d = points[i].distance(points[j]);
		points[i].wlDensity += pow(b, -(d * d)/sigma);
	    }
	    meanPointDensities[(*it).first] += points[i].wlDensity;
	    for(map<int, vector<HybPoint> >::iterator jt=hybPoints.begin(); jt != hybPoints.end(); jt++){
		vector<HybPoint>& pts = (*jt).second;
		for(uint j=0; j < pts.size(); j++){
		    double d = points[i].distance(pts[j]);
		    points[i].density += pow(b, -(d * d)/sigma);
		}
	    }
	    meanPointDensity += points[i].density;
	}
	meanPointDensities[(*it).first] /= double((*it).second.size());
    }
    meanPointDensity /= double(totalCount);
}

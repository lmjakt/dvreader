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

#include "space.h"
#include <iostream>
#include <stdlib.h>

using namespace std;


Row::~Row()
{
    while(pnts.size()){
	delete pnts.begin()->second;
	pnts.erase(pnts.begin());
    }
}

Point* Row::insertPoint(int x, int y, int z){
    Point* p = new Point(x, y, z);
    pnts.insert(make_pair(x, p));
    return(p);
}

vector<Point*> Row::points(int xb, int xe)
{
    vector<Point*> p;
    if(xb > xe)
	return(p);   // silently fail
    for(multimap<int, Point*>::iterator it = pnts.lower_bound(xb); it != pnts.lower_bound(xe); it++)
	p.push_back(it->second);
    return(p);
}

Plane::~Plane()
{
    while(rows.size()){
	delete rows.begin()->second;
	rows.erase(rows.begin());
    }
}

Point* Plane::insertPoint(int x, int y, int z)
{
    if(!rows.count(y))
	rows.insert(make_pair(y, new Row()));
    return(rows[y]->insertPoint(x, y, z));
}

vector<Point*> Plane::points(int xb, int xe, int yb, int ye)
{
    vector<Point*> p;
    if(xb > xe || yb > ye)
	return(p);
    for(map<int, Row*>::iterator it = rows.lower_bound(yb); it != rows.upper_bound(ye); it++){
	vector<Point*> tp = it->second->points(xb, xe);
	p.insert(p.end(), tp.begin(), tp.end());
    }
    return(p);
}

Space::~Space()
{
    while(planes.size()){
	delete planes.begin()->second;
	planes.erase(planes.begin());
    }
}

Point* Space::insertPoint(int x, int y, int z)
{
    if(!planes.count(z))
	planes.insert(make_pair(z, new Plane()));
    return(planes[z]->insertPoint(x, y, z));
}

vector<Point*> Space::points(int xb, int xe, int yb, int ye, int zb, int ze)
{
    vector<Point*> p;
    if(xb > xe || yb > ye || zb > ze){
	cerr << "Space::points invalid arguments : " << xb << "," << xe << " : " << yb << "," << ye << " : " << zb << "," << ze << endl;
	return(p);
    }
    for(map<int, Plane*>::iterator it = planes.lower_bound(zb); it != planes.upper_bound(ze); it++){
	vector<Point*> tp = it->second->points(xb, xe, yb, ye);
	p.insert(p.end(), tp.begin(), tp.end());
    }
    return(p);
}
				      
vector<Point*> Space::points(int x, int y, int z, int radius)
{
  radius = abs(radius);
  return(points(x - radius, x + radius, y - radius, y + radius, z - radius, z + radius));
}
  

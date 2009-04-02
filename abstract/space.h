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

#ifndef SPACE_H
#define SPACE_H

// This defines points in 3 dimensional space, where the density of spots is relatively sparse.
// It uses a number of functions to return points within a given subspace, -without knowing much
// about that space

// Note that these points will allow certain parameters to be included.

#include <vector>
#include <map>
#include <set>

struct Point {    // 
    int id;
    int x, y, z;
    Point* parent; // only one parent allowed.. This is important for DAGs
    std::set<Point*> daughters;

    Point(){
	id = -1;
	parent = 0;
    }
    Point(int X, int Y, int Z){
	x = X; y = Y; z = Z;
	parent = 0;
	id = -1;
    }
    friend bool operator <(const Point& a, const Point& b){
	if(a.z != b.z)
	    return(a.z < b.z);
	if(a.y != b.y)
	    return(a.y < b.y);
	return(a.x < b.x);
    }
    ~Point(){}
};

struct Row {
    std::multimap<int, Point*> pnts;
    Point* insertPoint(int x, int y, int z);
    std::vector<Point*> points(int xb, int xe);
    Row(){}
    ~Row();
};

struct Plane {
    std::map<int, Row*> rows;
    Plane(){}
    ~Plane();
    Point* insertPoint(int x, int y, int z);
    std::vector<Point*> points(int xb, int xe, int yb, int ye);  // do not use pointers if space is deleted
};

class Space {
    std::map<int, Plane*> planes;
    
    public :
	Space(){}  // needs no initialisation.. 
    ~Space();      // have to delete
    Point* insertPoint(int x, int y, int z);
    std::vector<Point*> points(int xb, int xe, int yb, int ye, int zb, int ze);

};

#endif

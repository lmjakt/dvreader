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

#ifndef SPOTPERIMETERMAPPER_H
#define SPOTPERIMETERMAPPER_H

#include "../../abstract/space.h"
#include "../../dataStructs.h"
#include "../perimeter.h"
#include <vector>
#include <set>
#include <QObject>

class SpotPerimeterMapper : public QObject
{
    Q_OBJECT
	
	public :
	SpotPerimeterMapper(int md);   // take the maximum distance as an argument
    ~SpotPerimeterMapper();
    std::vector<Point> findPerimeter(std::vector<simple_drop*>& points);         
    std::vector<Point> findPerimeter(std::vector<simple_drop*>& points, Perimeter& nuclearPerimeter, unsigned int interval);         
    std::vector<Point> findPerimeter(std::vector<Point*>& points, int maxD);   // we don't care, as long as the points are ok ? 

    public slots :
	void setMaxDistance(int md);
    
    private :
	double angleFromXaxis(Point* o, Point* p);    // though I think I don't actually use this ..
    double angleBetweenPoints(Point* pp, Point* cp, Point* np);
    Point* findNextPerimeterPoint(std::vector<Point*>& points, Point* cp, Point* pp, int maxD);
    bool vectorsCross(Point* cp, Point* np);    // check if it crosses any of the old vectors.. 
    bool vectorsCross(Point* a1, Point* a2, Point* b1, Point* b2);  // whether or not the lines cross.. 

    std::vector<Point*> perimeter;
    std::set<Point*> perimeterSet; 
    int maxDistance;
    void rotate(double& x, double& y, double cos_a, double sin_a){
	double xo = x; double yo = y;
	x = xo * cos_a - yo * sin_a;
	y = xo * sin_a + yo * cos_a;
    }
    
};

#endif

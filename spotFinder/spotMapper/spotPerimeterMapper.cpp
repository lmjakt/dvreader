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

#include "spotPerimeterMapper.h"
#include <iostream>

using namespace std;

SpotPerimeterMapper::SpotPerimeterMapper(int md)
{
    maxDistance = md;
}

SpotPerimeterMapper::~SpotPerimeterMapper()
{
}

void SpotPerimeterMapper::setMaxDistance(int md)
{
    maxDistance = md;
    if(maxDistance <= 0)
	maxDistance = 2;       // one is just too silly .. 
}

vector<Point> SpotPerimeterMapper::findPerimeter(vector<simple_drop*>& td_points)
{
    vector<Point*> points(td_points.size());
    for(uint i=0; i < td_points.size(); i++)
	points[i] = new Point(td_points[i]->x, td_points[i]->y, td_points[i]->z);

    cout << "calling findPerimeter with points size " << points.size() << endl;
    vector<Point> pPoints = findPerimeter(points, maxDistance);


    // after doing something reasonable with the pPoints delete things..
    for(uint i=0; i < points.size(); ++i)
	delete points[i];
    return(pPoints);
}

vector<Point> SpotPerimeterMapper::findPerimeter(vector<simple_drop*>& td_points, Perimeter& nuclearPerimeter, unsigned int interval)
{
    vector<Point*> points;
    points.reserve(td_points.size() + nuclearPerimeter.length() / interval);  // 1 cause I'm not sure of my ari
    
    // make points from the perimeter at every interval position..
    for(uint i=0; i < nuclearPerimeter.length(); i += interval){
	int x, y;
	if(nuclearPerimeter.pos(i, x, y))
	    points.push_back(new Point(x, y, 0));
    }
    for(uint i=0; i < td_points.size(); ++i)
	points.push_back(new Point(td_points[i]->x, td_points[i]->y, td_points[i]->z));
    
    vector<Point> pPoints = findPerimeter(points, maxDistance);
    for(uint i=0; i < points.size(); ++i)
	delete points[i];
    return(pPoints);
}

vector<Point> SpotPerimeterMapper::findPerimeter(vector<Point*>& points, int maxD)
{
    // First find the point with the smallest y value, and make this the starting point.
    // For the first round, assume that the previous point lies directly to the right of the starting point.
    // Then find the next point with the closest angle..
    
    perimeter.resize(0);
    perimeterSet.clear();
    vector<Point> perimeterCopy;
//    vector<Point> perimeter;
    if(!points.size())
	return(perimeterCopy);
    Point* sp = points[0];
    for(uint i=1; i < points.size(); i++){
	if(sp->y > points[i]->y)
	    sp = points[i];
    }
    cout << "Starting with a point with address : " << (long)sp << "  position : " << sp->x << "," << sp->y << endl;
    perimeter.push_back(sp);
    // but don't insert into the set, as returning to the starting point is OK.
    Point psp(sp->x + 1, sp->y, sp->z);  // the pseudopoint
    Point* pp = &psp;
    Point* cp = sp;
    Point* np = 0;

    int max_max = 400;
    while(np != sp && perimeter.size() < points.size()){
	int max_d = maxD;
	while( !( np = findNextPerimeterPoint(points, cp, pp, max_d) ) ){
	    max_d += maxD;
	    cout << "np is 0 so we will inccrease max_d to : " << max_d << endl;
	    if(max_d > max_max)
		break;
	}
	if(np){
	    np->parent = cp;
	    perimeter.push_back(np);
	    perimeterSet.insert(np);
	}
	if(max_d > max_max){        // we are stuck, try retreating backwards.. 
	    cout << "max_d is above max_max either giving up or going backwards" << endl;
	    if(pp->parent){   
		cout << "\tretreating one step" << endl;
		perimeter.push_back(pp);  // this way we go backwards when drawing
		cp = pp;
		pp = pp->parent;
		continue;
	    }
	    cout << "\tgiving up" << endl;
	    break;
	}
	pp = cp;
	cp = np;
    }
    cout << "and made a perimeter with size : " << perimeter.size() << endl;
    perimeterCopy.resize(perimeter.size());
    for(uint i=0; i < perimeter.size(); ++i)
	perimeterCopy[i] = *perimeter[i];
    return(perimeterCopy);
}


double SpotPerimeterMapper::angleFromXaxis(Point* o, Point* p)
{
    // If point o is considered to lie at the origin of a circle, and the vector
    // from o to p (p - o) is scaled to have a magnitude of 1 then :
    // 
    // sin P = -dy    (-y since y is for the anti - clockwise angle.. )
    // cos P = dx
    // if both are calculated it is possible to determine the angle and the directionality of it
    
    double dx = (double)(p->x - o->x);
    double dy = (double)(o->y - p->y);  // i.e. - dy .. (we could also change this by 
    // then scale dx and dy accordingly. Do this the slow way to avoid making mistakes
    double l = sqrt( (dx * dx) + (dy * dy) );
    dx = dx / l;
    dy = dy / l;   // now the length of dx, dy is 1.

    // if dy is positive return acos(dx)
    // otherwise return M_PI + (M_PI - acos(dx)) or 2 * M_PI - acos(dx);
    if(dy >= 0)
	return(acos(dx));
    return(2 * M_PI - acos(dx));
}

// pp previous point, cp current point, np next point 
double SpotPerimeterMapper::angleBetweenPoints(Point* pp, Point* cp, Point* np)
{
    // Find the clockwise angle made from the vector pp, cp, np
    // i.e. imagine a circle with the origin at cp, and lines np and pp joining this circle.
    
    // Find the angle from vector pp -> cp and np -> cp.
    // Follow this method:
    // First scale both vectors to have a magnitude of 1. (This makes sins and cosines kind of easy).
    // Rotate both vectors anti-clockwise such that cp -> pp lies along the x-axis. 
    // That is rotate both vectors by the clockwise angle from the x-axis to the vector cp -> pp.
    // Then simply calculate the angle by the x and y coordinates ..
//    cout << "\ndetermining angle between : " << pp->x << "," << pp->y << " ->  " << cp->x << "," << cp->y << "  ->  " << np->x << "," << np->y << endl;

    double pp_x = (double)(pp->x - cp->x);
    double pp_y = (double)(pp->y - cp->y);
    double m = sqrt( (pp_x * pp_x) + (pp_y * pp_y) );
    pp_x /= m;
    pp_y /= m;
    
    double np_x = (double)(np->x - cp->x);
    double np_y = (double)(np->y - cp->y);
    m = sqrt( (np_x * np_x) + (np_y * np_y) );
    np_x /= m;
    np_y /= m;

    //   cout << "Positions transformed to : " << pp_x << "," << pp_y << "  --> " << np_x << "," << np_y << endl;

    // In this case, if we call the clockwise angle from the x-axis to the vector cp -> pp as a
    // we can say that :
    // cos(a) = pp_x
    // sin(a) = -pp_y     (negative since the angle is clockwise rather than anticlockwise).
    
    // To rotate anti - clockwise the vector cp -> np by the angle a we use the standard rotation matrix :
    // |x'| = |cos(a)  -sin(a)| |x|
    // |y'|   |sin(a)   cos(a)| |y|
    // 
    // which is a bit difficult to read but which ends up being :
    // x' = x * cos(a) - y * sin(a)
    // y' = x * sin(a) + y * cos(a)
    //
    // However, since for this special occasion :
    // cos(a) = pp_x
    // sin(a) = -pp_y;
    // and x = np_x and y = np_y
    // x' = np_x * pp_x - (np_y * -pp_y)
    // y' = np_x * -pp_y + (np_y * pp_x)
    double x = np_x * pp_x - (np_y * -pp_y);
    double y = np_x * -pp_y + (np_y * pp_x);
//    cout << "and after rotation new point is : " << x << "," << y << "  so the angle will be : " << acos(x) / M_PI << "  or  " << (2 * M_PI - acos(x))/ M_PI << endl;
    // and then simply : 
    if(-y > 0)
	return(acos(x));
    return(2 * M_PI - acos(x));
}

Point* SpotPerimeterMapper::findNextPerimeterPoint(vector<Point*>& points, Point* cp, Point* pp, int maxD)
{
    Point* np = 0;   // the next point.. 
    maxD *= maxD;    // then we don't have to call sqrt
    double minAngle = 4 * M_PI;   // all angles will be less than this..
    double min_angle = M_PI / 50.0;   // about 3.6 degree. To avoid rounding errors primarily. (but might be bad)
    int minChoiceNo = 5;
    int choiceNo = 0;   // the number of points within striking distance.. 
    for(uint i=0; i < points.size(); i++){
	if(points[i] == cp || points[i] == pp)
	    continue;
	if( ((points[i]->x - cp->x) * (points[i]->x - cp->x) + (points[i]->y - cp->y) * (points[i]->y - cp->y) ) > maxD)
	    continue;
	if(perimeterSet.count(points[i]))
	    continue;
	++choiceNo;
	double angle = angleBetweenPoints(pp, cp, points[i]);
	if(angle < minAngle && angle > min_angle && !vectorsCross(cp, points[i])){
	    minAngle = angle;
	    np = points[i];
	}
    }
    if(choiceNo < minChoiceNo){  // the point choice is not safe
	np = 0;
    }
//    if(np)
//	cout << "obtained a new point with address : " << (long)np << "  and position : " << np->x << "," << np->y << "   angle : " << minAngle << endl;
    return(np);
}

// check if the line between cp and np crosses any of the lines already in the thingy
bool SpotPerimeterMapper::vectorsCross(Point* cp, Point* np)
{
//    cout << "Checking if vectors cross perimeter size is " << perimeter.size()  << endl;
    for(uint i=1; i < perimeter.size() - 1; ++i){  // the new segment will always cross with the last segment since they share a point
	if(vectorsCross(cp, np, perimeter[i-1], perimeter[i])){
//	    cout << "Vectors determined to cross : " << endl;
	    return(true);
	}
    }
    return(false);
}

bool SpotPerimeterMapper::vectorsCross(Point* a1, Point* a2, Point* b1, Point* b2)
{
// Unfortunately, the first method I tried beneath doesn't work: this is due to the obvious reason that
// the equation
//                y = ax + k
//
// obviously cannot support vertical lines, and is likely to lead to rounding errors for very steep lines. That's bad..
// See the new and more ugly way of doing this beneath the dashed lines

//     // This is kind of ugly, but determine the equations and then calculate the x value with the cross point.. 
//     // If that is within both the ranges then the lines cross .. 

//     // consider two equations : y = gx + k
//     // where g is the gradient and k is a constant
//     // than ag and ak for the line running through the points a1 and a2 is :
    
//     double ag = double(a1->y - a2->y)/double(a1->x - a2->x);
//     double ak = double(a1->y) - (double(a1->x) * ag);
//     // and for the line running through b1 and b2
//     double bg = double(b1->y - b2->y)/double(b1->x - b2->x);
//     double bk = double(b1->y) - (double(b1->x) * bg);

//     // if parallel, then lines never cross..
//     if(ag == bg){
// 	if(ak == bk)
// 	    return(true);
// 	return(false);
//     }
    
//     // otherwise the x value for the intersect can be defined as :
//     //    double xi = (ak - bk) / (ag - bg);
//     double xi = (bk - ak) / (ag - bg);
//     return( ( (xi <= (double)a1->x) != (xi <= (double)a2->x) )
// 	    &&
// 	    ( (xi <= (double)b1->x) != (xi <= (double)b2->x) ) ); 
//     // which is a short way of saying is xi within the line from a1 to a2 without knowing which x value is larger.


//   ====================================================================================================
//   OK try again.
//
//   This time try the following. 

//   First check if an overlap is possible by making sure that both the x and y axis overlap
//   then:
//   Translate all the points so that a1 lies at the origin. Then determine the cos and sin values for the
//   clockwise angle for the line a1 -> a2. Then rotate all points such that the line a1 -> a2 lies along the x-axis
//   Then if b1 and b2 lie on either side of the x axis and the x intercept of the line containing the thing is within the
//   the range.. then it's considered to be crossing. But oh my god, that is a right pain in the arse.. 

//  check if the lines are in line with each other...
    if( ((b1->x > a1->x) == (b1->x > a2->x) && (b2->x > a1->x) == (b2->x > a2->x) && (b1->x > a1->x) == (b2->x > a1->x) )
	||
	((b1->y > a1->y) == (b1->y > a2->y) && (b2->y > a1->y) == (b2->y > a2->y) && (b1->y > a1->y) == (b2->y > a1->y) )
	)
	return(false);
    

// get some variables for the transformed coordinates..
//    double a1x = 0.0;
//    double a1y = 0.0;
    double a2x = double(a2->x - a1->x);
    double a2y = double(a2->y - a1->y);
    double b1x = double(b1->x - a1->x);
    double b1y = double(b1->y - a1->y);
    double b2x = double(b2->x - a1->x);
    double b2y = double(b2->y - a1->y);
/// call the scaled version of a2x and a2y as ax and ay..
    double am = sqrt( (a2x * a2x) + (a2y * a2y) );
    // this are nice, because the clockwise sin and cosine are now simply
    // cos(a) = ax;
    // sin(a) = -ay;

    double cos_a = a2x / am;
    double sin_a = -(a2y / am);


    // Then rotate all the points above (apart from a1x ofcourse.. )..
    // by using the rotation transformation which is :
    // x' = x * cos(a) - y * sin(a)
    // y' = x * sin(a) + y * cos(a)
    
    rotate(a2x, a2y, cos_a, sin_a);
    rotate(b1x, b1y, cos_a, sin_a);
    rotate(b2x, b2y, cos_a, sin_a);
    
    // first to avoid any more trouble, just check if the b1_y and b2_y are of opposite values..
    if(b1y > 0 == b2y > 0)
	return(false);

    // so the b line crosses the x-axis, but we at this point don't know where, but since we have the points for it, it should 
    // not be a problem to solve. Note however, that the bline can still be vertical, so we need to express the line as
    // x = ay + b
    // but this shouldn't be too much of a problem.. 
    double a = (b1x - b2x) / (b1y - b2y);
    // and b = x - ay
    double b = b1x - a * b1y;
    // and if y = 0, then x is obviously equal to the term b.
    // if b is larger than 0 and smaller than a2x then we have an intercept. 
    return( b >= 0 && b <= a2x );
    // incredible . to go through so many lines of code to test if two line segments intercept. Terrible I'd say, but there you go.
}

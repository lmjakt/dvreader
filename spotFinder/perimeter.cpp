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

#include "perimeter.h"
#include <iostream>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <algorithm>


using namespace std;


void Perimeter::printRange(){
    cout << "\nPerimeter::printRange  " << minX << " -> " << maxX << "  :: " << minY << " --> " << maxY << endl;
}

int countIntersects(char* mask, char b, int xb, int yb, int xe, int ye, int w, int& oddCount, int& evenCount){
    // b is the boundary value
    // xb, yb is the starting position
    // xe, ye are the end positions 
    // w is the width of the mask..

    /// LIMITATION LIMIATATION .. 
    /// this only works for vertical and horizontal directions.. 
    /// for diagonals, there is a 50% chance of missing the boundary as it is not
    /// necessarily filled in.. 

    // first determine the direction we want to go in..
    
    int xd = xe > xb ? 1 : -1;
    int yd = ye > yb ? 1 : -1;
    
    xd = xe == xb ? 0 : xd;
    yd = ye == yb ? 0 : yd;  // important.. 

    // and then while x and y are not equal to xe or ye then keep going..
    int x = xb;
    int y = yb;
    int count = 0;
    char last = 0;
    while(x != xe || y != ye){
	if(mask[y * w + x] & b && !(last & b)){   // i.e. a bitwise and (so we can use more complicated masks..
	    ++count;
	}
	last = mask[y * w + x];
	x += xd;
	y += yd;
    }
    // update the values of oddCount or evenCount as appropriate..
    if(count % 2){
	++oddCount;
    }else{
	++evenCount;
    }
    return(count);  // and then we don't need to use the count variable.. 
}    


Perimeter::Perimeter(vector<QPoint> points, unsigned int gw, unsigned int gh)
{
  globalWidth = gw;
  globalHeight = gh;
  minX = maxX = minY = maxY = 0;
  if(!points.size()) return;
  minX = maxX = points[0].x();
  minY = maxY = points[0].y();
  perimeter.reserve(points.size());
  for(unsigned int i=0; i < points.size(); ++i){
    perimeter.push_back(points[i].y() * globalWidth + points[i].x());
    minX = minX > points[i].x() ? points[i].x() : minX;
    maxX = maxX < points[i].x() ? points[i].x() : maxX;
    minY = minY > points[i].y() ? points[i].y() : minY;
    maxY = maxY < points[i].y() ? points[i].y() : maxY;
  }
  nucleusId = -1;
}

/*
  It seems that I might be able to remove isContainedInRegion, though it's quite good to keep it here
  as it illustrates how one can answer the questions for an irregular shape.. by counting boundary crossings
  But which fails when touching but not crossing a boundary. 
 */

bool Perimeter::isContainedInRegion(char* mask, char me, char her, int xo, int yo, int w, int h){
    cout << "\nISCONTAINEDINREGION..." << endl;
    // very simple takes a number of points along my perimeter, and makes 4 lines
    // left, right, bot, top
    // and counts the number of times we cross her boundary. Then adds up even and odd..
    int pno = 4;
    int oddCount, evenCount;
    oddCount = evenCount = 0;
    
    if(!perimeter.size()){
	return(false);
    }

    for(int i=0; i < pno; ++i){
	// new position is ..
	uint p = (i * perimeter.size()) / pno;
	int y = perimeter[p] / globalWidth - yo;
	int x = perimeter[p] % globalWidth - xo;
	// and then go in the three directions and count lines..
	countIntersects(mask, her, x, y, 0, y, w, oddCount, evenCount);
	countIntersects(mask, her, x, y, w-1, y, w, oddCount, evenCount);
	countIntersects(mask, her, x, y, x, 0, w, oddCount, evenCount);
	countIntersects(mask, her, x, y, x, h-1, w, oddCount, evenCount);
    }
    cout << "Perimeter::isContainedInRegion oddCount : " << oddCount << "\tevenCount : " << evenCount << endl;
    return(oddCount > evenCount);
}

char* Perimeter::makeMask(char bvalue, char outvalue, int& mw, int& mh, int& m_xo, int& m_yo){
    // Note outvalue must not be 0.. this will fail.. 
    mw = maxX - minX + 3;
    mh = maxY - minY + 3;  // one border pixel either side (+2) and +1 since it is an inclusive range.. 
    m_xo = minX - 1;
    m_yo = minY - 1;   // Note these values will often be less than 0. So take some care..

//    cout << "makeMask mw " << mw << " mh " << mh << endl
//	 << "minX " << minX << " maxX " << maxX << endl;

    char* m = new char[mw * mh];
    memset((void*)m, 0, mw * mh * sizeof(char));
    // and then for the magic..
    for(uint i=0; i < perimeter.size(); ++i){
	int x = perimeter[i] % globalWidth;
	int y = perimeter[i] / globalWidth;
	m[(y - m_yo) * mw + (x - m_xo)] = bvalue;
	// Diagonals are ok if we use the below floodFill algorithm. Since the flood fill doesn't
	// do diagonals we don't actually need to worry about it.. 
    }
//    cout << "calling flood fill " << endl;
//     for(int y=0; y < mh; y++){
// 	for(int x=0; x < mw; x++){
// 	    cout << (int)m[y * mw + x];
// 	}
// 	cout << endl;
//     }
//    cout << "makeMask before floodfill the value of begin and back : " << perimeter[0] << " --> " << perimeter.back() << endl;
    floodFill(m, mw, mh, bvalue, outvalue, 0, 0);   // 0, 0 is specified to be outside and have a null value..
    return(m);
}

void Perimeter::floodFill(char* mask, int mw, int mh, char bvalue, char outvalue, int x, int y){
    int p = y * mw + x;
//    cout << x << "," << y << "  :  " << mask[p] << endl;
    mask[p] = outvalue;
    // and recurse..
    if(x - 1 >= 0 && !mask[p-1])
	floodFill(mask, mw, mh, bvalue, outvalue, x-1, y);
    if(x + 1 < mw && !mask[p+1])
	floodFill(mask, mw, mh, bvalue, outvalue, x+1, y);
    if(y - 1 >= 0 && !mask[p-mw])
	floodFill(mask, mw, mh, bvalue, outvalue, x, y-1);
    if(y + 1 < mh && !mask[p + mw])
	floodFill(mask, mw, mh, bvalue, outvalue, x, y+1);
    // and that is it....
}

int Perimeter::findPos(char* mask, int mw, int mh, char fvalue, char bvalue, int x, int y){
    // this is a little confusing, but ..
    // fvalue is the value of the point we are looking for
    // bvalue is the value of a this boundary line
    // we must reach fvalue from the starting point without crossing any point that is not bvalue.. 

    // basically, we want to find the first point from the starting position with fvalue.. but not 
    // if that means crossing a bvalue point. Note that 
    // we can't use diagonals, since diagonal lines can cross other diagonal lines.. 
    
    int xoff[] = {0, 1, 0, -1};
    int yoff[] = {-1, 0, 1, 0};     // this gives us a clockwise spin around a central position.. 
    char ok[4];
    memset((void*)ok, 1, sizeof(char) * 4);        // use as a boolean array to check if allowable. (If we hit a boundary then not allowable)
    
    cout << "findPos fvalue : " << (int)fvalue << "  bvalue : " << (int)bvalue << endl;

    int m = 1;  // the mutliplier..
    int p = y * mw + x;
    bool go_on = true;
    while(go_on){
	go_on = false;
	for(int i=0; i < 4; ++i){
	    if(!ok[i])
		continue;
	    int nx = x + xoff[i] * m;
	    int ny = y + yoff[i] * m;
	    cout << i << "\tpos : " << nx << "," << ny << endl;
	    if(nx >= 0 && nx < x + mw && ny >= 0 && ny < y + mh){
		go_on = true;         // at least one of the directions is still ok..
		cout << "\t\t" << (int)mask[ny * mw + nx] << endl;
		if(mask[ny * mw + nx] == fvalue)
		    return(ny * mw + nx);
		if(mask[ny * mw + nx] != bvalue)  // don't go in this direction anymore.. 
		    ok[i] = 0;
	    }
	}
	++m;
    }
    return(-1);   // if we can't find anything good. Which should be fairly often. 
}

void printMask(char* mask, int mw, int mh){
    int y = mh;
    while(y > 0){
	--y;
	for(int x=0; x < mw; ++x){
	    cout << (int)mask[y * mw + x];
	}
	cout << endl;
    }
}

vector<vector<int> > Perimeter::splitPerimeters(vector<vector<int> >& splitLines){
    // Make a mask of the area, and then for each of the split lines fill in the area 
    // either side of the split line using the flood fill with a new value.
    int mw, mh, m_xo, m_yo;
    char inside = 0;
    char b = 1;  // the boundary value..
    char o = 2;
    char sl = 3;  // a split line value.. 
    cout << "making the mask for splitPerimeters" << endl;
    char* mask = makeMask(b, o, mw, mh, m_xo, m_yo);
    vector<int> midPoints(splitLines.size(), -1);

    /// And the perimeters that we will return ..
    vector<vector<int> > newPerimeters;
    // If we don't have any split lines, then simply return a perimeter for the current one
    if(!splitLines.size()){
	newPerimeters.push_back(perimeter);
	return(newPerimeters);
    }

    // and then add the splitLines to the mask..
    cout << "Perimeter::splitPerimeter about to check how we are going to add the split lines" << endl;
    for(uint i=0; i < splitLines.size(); ++i){
	cout << "examining split lines " << i << endl;
	// go from the mid point towards 0 or towards size
	// stop at first boundary.. 
	int begin, end, mid;
	begin = end = -1;
	mid = (int)splitLines[i].size() / 2;
	for(int j=mid; j >= 0; j--){
	    int dx = (splitLines[i][j] % globalWidth) - m_xo;
	    int dy = (splitLines[i][j] / globalWidth) - m_yo;
	    if(dx >= 0 && dx < mw && dy >= 0 && dy < mh && mask[dy * mw + dx])
		begin = j+1;
	    if(begin != -1)
		break;
	}
	cout << "Found a begin at position " << begin << endl;
	for(int j=mid; j < (int)splitLines[i].size(); j++){
	    int dx = (splitLines[i][j] % globalWidth) - m_xo;
	    int dy = (splitLines[i][j] / globalWidth) - m_yo;
	    if(dx >= 0 && dx < mw && dy >= 0 && dy < mh && mask[dy * mw + dx])
		end = j-1;
	    if(end != -1)
		break;
	}
	cout << "found an end at position " << end << endl;
	// that's particularly ugly, but .. then if we have a begin and an end we can do the next thing
	if(begin != -1 && end != -1){
	    midPoints[i] = (begin + end) / 2;
	    cout << "filling in mask for points " << begin << " --> " << end << endl;
	    for(int j=begin; j <= end; j++){
	    int dx = (splitLines[i][j] % globalWidth) - m_xo;
	    int dy = (splitLines[i][j] / globalWidth) - m_yo;
	    mask[dy * mw + dx] = sl;
	    }
	}else{
	    midPoints[i] = -1;
	}
    }
    char l = sl;
//    printMask(mask, mw, mh);  // see what it looks like.. 
    // At this point we should have a mask, where the original boundary elements are drawn, with the splitLines filled in.
    // What we want to do now is to use flood fill to fill in the area on either side of each split line and give each area its own number..
    for(uint i=0; i < splitLines.size(); ++i){
	cout << "Attempting flood fill for split line " << i << endl;
	if(midPoints[i] == -1){
	    cerr << "splitting perimeter up, but line " << i << " does not seem to fall in the appropriate location" << endl;
	    continue;
	}
	// then find a point one one side and fill in from there using 
	int x = (splitLines[i][midPoints[i]] % globalWidth) - m_xo;
	int y = (splitLines[i][midPoints[i]] / globalWidth) - m_yo;
	int p = findPos(mask, mw, mh, inside, sl, x, y);
	// then simply do the mask on that one..
	cout << "The midpoint is " << x << "," << y << endl;
	cout << "p is " << p << " : " << p % mw << "," << p/mw << endl;
	cout << "mw : " << mw << "\tmh " << mh << endl;
	if(p != -1){
	    ++l;
	    floodFill(mask, mw, mh, b, l, p % mw, p / mw);
	}
//	printMask(mask, mw, mh);
	// then find p again ..
	p = findPos(mask, mw, mh, inside, sl, x, y);
	cout << "and the second p is " << p << " : " << p % mw << "," << p/mw << endl;
	if(p != -1){
	    ++l;
	    floodFill(mask, mw, mh, b, l, p % mw, p / mw);
	}
    }
    cout << "p is done " << endl;
    // At this point we have got a number of floodfilled areas that we want to trace around. I might have to write another tracing function for this.
    // This should be straightforward. The first value of l is 3, and this goes up to the value of l..

    for(char k=sl + 1; k <= l; k++){
	cout << "calling tracePerimeter .. on " << (int)k << endl;
	newPerimeters.push_back(tracePerimeter(mask, mw, mh, m_xo, m_yo, k));
    }
    delete mask;
    // if that didn't give us anything, then default to the original perimeter..
    if(!newPerimeters.size()){
	newPerimeters.push_back(perimeter);
    }
    return(newPerimeters);
}

vector<QPoint> Perimeter::qpoints()
{
  vector<QPoint> qp;
  qp.reserve(perimeter.size());
  cout << "Size of qp : " << qp.size() << endl;
  for(unsigned int i=0; i < perimeter.size(); ++i){
    QPoint p(perimeter[i] % globalWidth, perimeter[i] / globalWidth );
    qp.push_back(p);
  }
  return(qp);
}

vector<int> Perimeter::tracePerimeter(char* m, int w, int h, int ox, int oy, char bc){
    // find an entrance point..
    bool b = false;
    int sx, sy;
    sx = sy = 0;
    for(int y=0; y < h; ++y){
	for(int x=0; x < w; ++x){
	    if(m[y * w + x] == bc){
		sx = x;
		sy = y;
		b = true;
		break;
	    }
	}
	if(b)
	    break;
    }
    // lets not do any error checking.. but let's just blithely get on with it..
    int xoffsets[] = {-1, 0, 1, 1, 1, 0, -1, -1};
    int yoffsets[] = {1, 1, 1, 0, -1, -1, -1, 0};     // this gives us a clockwise spin around a central position.. 
    int offset_offsets[] = {5, 6, 7, 0, 1, 2, 3, 4};  //    
    int offsetPos = 0;
    
    vector<int> per;
    per.reserve(perimeter.size());

    int x = sx;
    int y = sy;
    int origin = y * w + x;
    per.push_back((y + oy) * globalWidth + x + ox);
    bool keep_going = true;
    while(keep_going){
	for(int i=0; i < 8; i++){
	    int op = (offsetPos + i) % 8;
	    int nx = x + xoffsets[op];
	    int ny = y + yoffsets[op];
	    int np = ny * w + nx;
	    if(np == origin){
		keep_going = false;
		break;
	    }
	    // make sure that the new position is within bounds..
	    if(nx < 0 || nx >= w || ny < 0 || ny >= h){
		continue;
	    }
	    // then check if this position satisfies the criteria..
	    if(m[np] == bc){
		per.push_back((ny + oy) * globalWidth + nx + ox);
		// and set up the next offsetPos.
		offsetPos = offset_offsets[op];
		x = nx;
		y = ny;
		break;
	    }
	}
    }
    // and at this point we make a thingy and another thingy.. 
    return(per);

}

unsigned int Perimeter::area(){
    if(!areaPoints.size())
	setDetails();
    return(areaPoints.size());
}

bool Perimeter::contains(int x, int y){
    int p = y * globalWidth + x;
    if(!areaPoints.size())
	setDetails();
    return(areaPoints.count(p));
}

void Perimeter::centerPos(int& x, int& y){
    if(!areaPoints.size())
	setDetails();
    x = centerPosition % globalWidth;
    y = centerPosition / globalWidth;
}

int Perimeter::minSqDistance(int max, int x, int y){
    int d = (int)sqrt((double)max);
    int minD = max + 1;
    if((x < (minX - d) || x > (maxX + d))
       &&
       (y < (minY - d) || y > (maxY + d)) )   // i.e. lies outside the bounding box expanded by d
      return(minD);

    for(uint i=0; i < perimeter.size(); ++i){
	int px = perimeter[i] % globalWidth;
	int py = perimeter[i] / globalWidth;
	if( ((px - x)*(px - x) + (py - y)*(py - y)) < minD)
	    minD = ((px - x)*(px - x) + (py - y)*(py - y));
    }
    return(minD);
}

bool Perimeter::sqDistanceLessThan(int sqd, int x, int y){
    // first check the distance from the maxX, maxY etc lines...
    int d = (int)sqrt((double)sqd);
    if( 
	(abs(x - minX) > d &&
	 abs(x - maxX) > d)
	||
	(abs(y - minY) > d &&
	 abs(y - maxY) > d)
	)
	return(false);
    // otherwise calculate the distance from each point and if it is smaller than the given point
    // return the distance..
    for(uint i=0; i < perimeter.size(); ++i){
	int px = perimeter[i] % globalWidth;
	int py = perimeter[i] / globalWidth;
	if( ((px - x)*(px - x) + (py - y)*(py - y)) <= d)
	    return(true);
    }
    return(false);
}

void Perimeter::setDetails(){
    // this function determines the points within the perimeter and the final area..
    // by using a mask..
    char bvalue = 1;
    char outvalue = 2;
    int mw, mh, m_xo, m_yo;
    char* mask = makeMask(bvalue, outvalue, mw, mh, m_xo, m_yo);
    // This mask will have the non area contining the outvalue, and the inside containing
    // 0 .. and hence can be used to insert things..
    int xm = 0;
    int ym = 0;           // the mean or center positions
    areaPoints.clear();   // just in case..

    for(int y=0; y < mh; ++y){
	for(int x=0; x < mw; ++x){
	    if(mask[y * mw + x] != outvalue){
		areaPoints.insert( (y + m_yo) * globalWidth + x + m_xo );
		xm += x + m_xo;
		ym += y + m_yo;
	    }
	}
    }
    xm = xm / areaPoints.size();
    ym = ym / areaPoints.size();
    centerPosition = ym * globalWidth + xm;
    delete mask;
}


PerimeterSet::PerimeterSet(const PerimeterSet& that){
    refCounter = that.refCounter;
    (*refCounter)++;
    perimeters = that.perimeters;
    outlinePerimeter = that.outlinePerimeter;
    mask = that.mask;
    mask_ox = that.mask_ox;
    mask_oy = that.mask_oy;
    mask_w = that.mask_w;
    mask_h = that.mask_h;
    values = that.values;
    bvalue = that.bvalue;
    outvalue = that.outvalue;
    selectedPerimeters = that.selectedPerimeters;
}

PerimeterSet& PerimeterSet::operator=(const PerimeterSet& that){
    if(this != &that){
	(*refCounter)--;
	if(!*refCounter){
	    delete refCounter;
	    delete mask;
	    delete values;
	}
	refCounter = that.refCounter;
	(*refCounter)++;
	perimeters = that.perimeters;
	outlinePerimeter = that.outlinePerimeter;
	mask = that.mask;
	mask_ox = that.mask_ox;
	mask_oy = that.mask_oy;
	mask_w = that.mask_w;
	mask_h = that.mask_h;
	values = that.values;
	bvalue = that.bvalue;
	outvalue = that.outvalue;
	selectedPerimeters = that.selectedPerimeters;
    }
    return(*this);
}

PerimeterSet::~PerimeterSet(){
    (*refCounter)--;
    if(!*refCounter){
	delete refCounter;
	delete mask;
	delete values;
    }
}

bool PerimeterSet::addPerimeter(Perimeter per, float* source){
    if(!perimeters.size()){
	perimeters.push_back(per);
	outlinePerimeter = per;
	initialize(source);
	return(true);
    }
    return(mergeMasks(per, source));
}

void PerimeterSet::initialize(float* source){
    // This assumes that we have already go a perimeter and that this is the same as the 
    // outline perimeter..
    
    // first make a mask for the outline perimeter. Since this is the first time we do this
    // we can just use the mask and the values thing. Note that the mask will have a one pixel 
    // border so we have to take care in filling int those values.. 
    
    // The purpose of keeping the mask is that it is easy then to check whether or not something overlaps with that mask,
    // and that is much faster than checking if something overlaps using the perimeter based methods. But at the same time
    // I don't want each perimeter to keep its own mask, that might end up being expensive. Hence we do it in this class
    // and not in the Perimater class as I tried to do it before..

    mask = outlinePerimeter.makeMask(bvalue, outvalue, mask_w, mask_h, mask_ox, mask_oy);
    // and now that I have this I can fill the values with stuff.. 
    values = new float[mask_w * mask_h];
    memset((void*)values, 0, mask_w * mask_h * sizeof(float));
    // and then simply ..
    for(int dy=1; dy < mask_h; ++dy){
	for(int dx=1; dx < mask_w; ++dx){
	    if(mask[dy * mask_w + dx] != outvalue){
		values[dy * mask_w + dx] = source[(dy + mask_oy) * outlinePerimeter.globalWidth + dx + mask_ox];
	    }
	}
    }
    setPerimeterParameters(perimeters[0], mask, mask_ox, mask_oy, mask_w, mask_h);
    /// which is all I really need to do. These things may need to be changed when adding a thingy.. 
}


bool PerimeterSet::mergeMasks(Perimeter per, float* source){
    // First of all check if there is any overlap between the containing rectangles
    if(!(per.minX < outlinePerimeter.maxX && per.maxX > outlinePerimeter.minX
	 &&
	 per.minY < outlinePerimeter.maxY && per.maxY > outlinePerimeter.minY)){
	return(false);
    }
    // int this case make another mask using different values.. and compare this one to the 
    // current mask to determine if there is a real overlap or not.. 
    int p_ox, p_oy, p_w, p_h;
    char* pMask = per.makeMask(bvalue, outvalue, p_w, p_h, p_ox, p_oy);

    // determine the olap positions..
    int o_minX = mask_ox > p_ox ? mask_ox : p_ox;
    int o_minY = mask_oy > p_oy ? mask_oy : p_oy;
    int o_maxX = mask_ox + mask_w < p_ox + p_w ? mask_ox + mask_w -1 : p_ox + p_w -1;
    int o_maxY = mask_oy + mask_h < p_oy + p_h ? mask_oy + mask_h -1: p_oy + p_h - 1;
    
    // and the superset positions.. 
    int s_minX = mask_ox < p_ox ? mask_ox : p_ox;
    int s_minY = mask_oy < p_oy ? mask_oy : p_oy;
    int s_maxX = mask_ox + mask_w > p_ox + p_w ? mask_ox + mask_w -1: p_ox + p_w -1;
    int s_maxY = mask_oy + mask_h > p_oy + p_h ? mask_oy + mask_h -1 : p_oy + p_h -1;

    int sw = s_maxX - s_minX + 1;
    int sh = s_maxY - s_minY + 1;  // since it is inclusive.. 

    /////////////////////
    // and if there is an overlap break;;
    bool olap_exists = false;
    for(int y = o_minY; y <= o_maxY; ++y){
	for(int x = o_minX; x <= o_maxX; ++x){
	    // regardless of anything else set the super set to be 0 if either is 0
	    // but determine overlap only if both are not outside.. 
	    if(mask[(y - mask_oy) * mask_w + x - mask_ox] != outvalue
	       &&
	       pMask[(y - p_oy) * p_w + x - p_ox] != outvalue){
		olap_exists = true;
	    }
	}
    }

    // if olap_exists is false, then we just delete the extra masks, and return false
    // if olap_exists is true then we have to trace the new outline perimeter from the sp_mask
    if(!olap_exists){
	delete pMask;
	return(false);
    }

    char* sp_mask = new char[sw * sh];
    float* sp_values = new float[sw * sh];
    memset((void*)sp_mask, outvalue, sw * sh * sizeof(char));
    memset((void*)sp_values, 0, sw * sh * sizeof(float));

    // if overlap exists then we'll set the sp_values, but only then..
    // do this in two loops to minimise large numbers of if this or that..
    for(int y=mask_oy; y < mask_oy + mask_h; ++y){
	for(int x=mask_ox; x < mask_ox + mask_w; ++x){
	    if(mask[(y-mask_oy) * mask_w + x - mask_ox] != outvalue){
		sp_mask[(y - s_minY) * sw + x - s_minX] = 0;
		sp_values[(y - s_minY) * sw + x - s_minX] = source[y * outlinePerimeter.globalWidth + x];
	    }else{
	    }
	}
    }
    for(int y=p_oy; y < p_oy + p_h; ++y){
	for(int x=p_ox; x < p_ox + p_w; ++x){
	    
	    if(pMask[(y-p_oy) * p_w + x - p_ox] != outvalue){
		sp_mask[(y - s_minY) * sw + x - s_minX] = 0;
		sp_values[(y - s_minY) * sw + x - s_minX] = source[y * outlinePerimeter.globalWidth + x];
	    }else{
	    }
	}
    }

    cout << "so we call trace perimeters" << endl;
    // and now we have a bloody problem of tracing bollocks..
    vector<int> sp_per = tracePerimeter(sp_mask, s_minX, s_minY, sw, sh, outlinePerimeter.globalWidth, 2);
    setPerimeterParameters(per, pMask, p_ox, p_oy, p_w, p_h);
    perimeters.push_back(per);
    delete mask;
    delete values;
    delete pMask;
    mask = sp_mask;
    values = sp_values;
    mask_ox = s_minX;
    mask_oy = s_minY;
    mask_w = sw;
    mask_h = sh;
    outlinePerimeter.perimeter = sp_per;
    // set the parameters for the perimeter..
    return(true);
}



vector<int> PerimeterSet::tracePerimeter(char* m, int ox, int oy, int w, int h, int gw, char out){

    // find an entrance point..
    bool b = false;
    int sx, sy;
    sx = sy = 0;
    for(int y=0; y < h; ++y){
	for(int x=0; x < w; ++x){
	    if(m[y * w + x] != out){
		sx = x;
		sy = y;
		b = true;
		break;
	    }
	}
	if(b)
	    break;
    }
    // lets not do any error checking.. but let's just blithely get on with it..
    int xoffsets[] = {-1, 0, 1, 1, 1, 0, -1, -1};
    int yoffsets[] = {1, 1, 1, 0, -1, -1, -1, 0};     // this gives us a clockwise spin around a central position.. 
    int offset_offsets[] = {5, 6, 7, 0, 1, 2, 3, 4};  //    
    int offsetPos = 0;
    
    vector<int> per;
    per.reserve(outlinePerimeter.perimeter.size() * 2);

    int x = sx;
    int y = sy;
    int origin = y * w + x;
    bool keep_going = true;
    while(keep_going){
	for(int i=0; i < 8; i++){
	    int op = (offsetPos + i) % 8;
	    int nx = x + xoffsets[op];
	    int ny = y + yoffsets[op];
	    int np = ny * w + nx;
	    if(np == origin){
		keep_going = false;
		break;
	    }
	    // make sure that the new position is within bounds..
	    if(nx < 0 || nx >= w || ny < 0 || ny >= h){
		continue;
	    }
	    // then check if this position satisfies the criteria..
	    if(m[np] != out){
		per.push_back((y + oy) * gw + x + ox);
		// and set up the next offsetPos.
		offsetPos = offset_offsets[op];
		x = nx;
		y = ny;
		break;
	    }
	}
    }
    // and at this point we make a thingy and another thingy.. 
    return(per);
}

void PerimeterSet::setPerimeterParameters(Perimeter& per, char* perimeterMask, int o_x, int o_y, int w, int h){
    // although we could get the perimeter itself to make the mask, it should alreay have been made.
    
    // THIS function is really ugly and should be a member of someone else, but I can't quite think clearly
    // enough today.

    PerimeterParameters* pars = &per.parameters;  // just for shorthand..
    // First the obvious ones..
    pars->length = (int)(per.perimeter.size());
    
    // Then find the center
    int cx = 0;
    int cy = 0;
    for(uint i=0; i < per.perimeter.size(); ++i){
	int x, y;
	per.pos(i, x, y);
	cx += x;
	cy += y;
    }
    cx /= per.perimeter.size();
    cy /= per.perimeter.size();  // i.e. the mean..
    pars->centerX = cx;
    pars->centerY = cy;
    // and then get the distances ..
    pars->centerDistances.resize(per.perimeter.size());
    pars->mean_cd = 0;
    pars->std_cd = 0;
    for(uint i=0; i < per.perimeter.size(); ++i){
	int x, y;
	per.pos(i, x, y);
	pars->centerDistances[i] = sqrtf(float( (x - cx)*(x - cx) + (y - cy)*(y-cy) ));
	pars->mean_cd += pars->centerDistances[i]; 
    }
    pars->mean_cd /= (float)per.perimeter.size();
    // and then work out the variance..
    for(uint i=0; i < pars->centerDistances.size(); ++i){
	pars->std_cd += (pars->centerDistances[i] - pars->mean_cd) * (pars->centerDistances[i] - pars->mean_cd);
    }
    pars->std_cd /= (float)(pars->centerDistances.size());
    pars->std_cd = sqrtf(pars->std_cd);
    // and then to get the percentiles sort the thingy..
    sort(pars->centerDistances.begin(), pars->centerDistances.end());  // and then we just have to work out the appropriate indices
    pars->cd_10 = pars->centerDistances[(pars->length * 10) / 100];
    pars->cd_50 = pars->centerDistances[(pars->length * 50) / 100];
    pars->cd_90 = pars->centerDistances[(pars->length * 90) / 100];
    
    // And then we need to get statistics on the basis of the area covered and stuff like that..
    // use the mask provided..
    vector<float> signals;
    pars->signalSum = 0;
    pars->area = 0;
    signals.reserve(w * h);
    for(int dy=0; dy < h; ++dy){
	int y = dy + o_y;
	for(int dx=0; dx < w; ++dx){
	    int x = dx + o_x;
	    int set_mask_pos = (y - mask_oy) * mask_w + (x - mask_ox);
	    int mask_pos = dy * w + dx;
	    // and then we just say..
	    if(perimeterMask[mask_pos] != outvalue){
		pars->signalSum += values[set_mask_pos];
		pars->area++;
		signals.push_back(values[set_mask_pos]);
	    }
	}
    }
    // and then simply
    sort(signals.begin(), signals.end());
    pars->signal_10 = signals[(pars->area * 10)/100];
    pars->signal_50 = signals[(pars->area * 50)/100];
    pars->signal_90 = signals[(pars->area * 90)/100];
    
    /// and that is it I think... 
}

void PerimeterSet::setSelection(vector<vector<int> > perims){
    selectedPerimeters.resize(0);
    // special case if perims size is 0, set the selected perimeters to be the outline perimeter..
    if(!perims.size()){
	selectedPerimeters.push_back(outlinePerimeter);
	return;
    }
    for(uint i=0; i < perims.size(); ++i){
	selectedPerimeters.push_back(Perimeter(perims[i], outlinePerimeter.globalWidth, outlinePerimeter.globalHeight));
    }
}

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

#ifndef PERIMETER_H
#define PERIMETER_H


#include <vector>
#include <set>
#include <QPoint>
#include "../abstract/space.h"

typedef unsigned int uint;

struct PerimeterParameters {
    int length;
    int area;
    float signalSum; // the sum of the intensities
    float signal_10, signal_50, signal_90;   // the 10th, 50th and 90th percentiles
    std::vector<float> centerDistances;      // the distances from the center
    int centerX, centerY;
    float cd_10, cd_50, cd_90;               // and the indicated percentiles from the center
    float mean_cd, std_cd;                   // the mean and the standard deviation of the centers
    // from these values we can calculate a number values that are related to the shape and size
    // of the objects spanned by the perimeters.
    PerimeterParameters(){
	length = area = 0;
	signalSum = signal_10 = signal_50 = signal_90 = 0;
	cd_10 = cd_50 = cd_90 = 100;
	mean_cd = std_cd = 0;
    }
};

class Perimeter {

    friend class PerimeterSet;                 // which is a lazy way to access the functions.. 
    friend class PerimeterWindow;
    friend class SpotMapperWindow;

    int minX, minY;   // the position of the left bottom corner
    int maxX, maxY;   // the top right corner (in this sense it is easier to convert things

    std::vector<int> perimeter; // the points of the perimeter in global coordinates (y * globalW + x);
    unsigned int globalWidth, globalHeight; // the global perimeters for translating positions
  
    PerimeterParameters parameters;
    // This function only makes sense if there is no overlap between the two thingies, otherwise it'll do weird things.. 
    bool isContainedInRegion(char* mask, char me, char her, int xo, int yo, int w, int h);  // a heuristic as to whether I'm contained within her
    // does not guarantee the correct answer.. but should be reasonably fast.. 
    // see source to work out who to use..
    std::set<int> areaPoints;
    int centerPosition;              // these two are not set by default, but will be calculated when either
                                     // setDetails(), centerPos(int, int) or contains(int, int) is called.
    
    public :
    Perimeter(){
      minX = minY = maxX = maxY = 0;
      globalWidth = globalHeight = 0;
    }
    bool pos(uint i, int& x, int& y){
      if(i < perimeter.size()){
	x = perimeter[i] % globalWidth;
	y = perimeter[i] / globalWidth;
	return(true);
      }
      x = y = 0;
      return(false);
    }
    
    unsigned int length(){
      return(perimeter.size());
    }
    
    Perimeter(std::vector<int> points, unsigned int gw, unsigned int gh, int minx, int miny, int maxx, int maxy){
      perimeter = points;
      globalWidth = gw;	
      globalHeight = gh;
      minX = minx;
      minY = miny;
      maxX = maxx;
      maxY = maxy;
      nucleusId = -1;
    }
    
    Perimeter(std::vector<int> points, unsigned int gw, unsigned int gh){
      perimeter = points;
      globalWidth = gw;
      globalHeight = gh;
      if(points.size()){
	minX = maxX = points[0] % gw;
	minY = maxY = points[0] / gw;
	for(uint i=0; i < points.size(); ++i){
	  int x = points[i] % gw;
	  int y = points[i] / gw;
	  if(x > maxX)
	    maxX = x;
	  if(x < minX)
	    minX = x;
	  if(y > maxY)
	    maxY = y;
	  if(y < minY)
	    minY = y;
	}
      }else{
	minX = minY = maxX = maxY = 0;
      }
      nucleusId = -1;
    }
    Perimeter(std::vector<QPoint> points, unsigned int gw, unsigned int gh);
    std::vector<int> perimeterPoints(){
      return(perimeter);
    }
    unsigned int area();
    bool contains(int x, int y);
    bool sqDistanceLessThan(int sqd, int x, int y);   // returns true if the square of the distance from point x & y is is less than sqd
    int minSqDistance(int max, int x, int y);         // returns maxD + 1 if no point less than or equal to maxD
    void setDetails();
    void centerPos(int& x, int& y);
    void printRange();
    char* makeMask(char bvalue, char outvalue, int& mw, int& mh, int& m_xo, int& m_yo);  // mw and mh passed by reference so that the caller can easily find out.. 
                                                                   // outvalue must not be 0. as this function is too lazy . 
    void floodFill(char* mask, int mw, int mh, char bvalue, char outvalue, int x, int y);
    int findPos(char* mask, int mw, int mh, char fvalue, char bvalue, int x, int y);
    std::vector<int> tracePerimeter(char* m, int w, int h, int ox, int oy, char bc);
    std::vector<std::vector<int> > splitPerimeters(std::vector<std::vector<int> >& splitLines);

    //Perimeter overlaps(Perimeter& p);  // checks if there is an overlap between the two, if such overlap returns the perimeter of the union
    int nucleusId;                   // This is an arbitrary id, that can be set by external sources. Not much point to make it private
                                     // since it isn't set by Perimeter functions anyway. Note that it will default to -1 indicating no known
                                     // Nucleus.
    int xmin(){ return minX; }
    int xmax(){ return maxX; }
    int ymin(){ return minY; }
    int ymax(){ return maxY; }
    int g_width(){ return globalWidth; }
    int g_height(){ return globalHeight; }
    std::vector<QPoint> qpoints();
};

class PerimeterSet { // contains a set of overlapping perimeters as well as the complete perimeter as determine by thingy..
    friend class PerimeterWindow;  // though this class doesn't know what that is ?
    friend class SpotMapperWindow;
    public :
	PerimeterSet(){
	mask = 0;
	values = 0;
	bvalue = 1;
	outvalue = 2;
	refCounter = new int;
	*refCounter = 1;
    }
    ~PerimeterSet();
    // since this contains dynamically allocated memory I'll need
    // a copy constructor and an operator= function as well
    PerimeterSet(const PerimeterSet& that);
    PerimeterSet& operator=(const PerimeterSet& that);


    bool addPerimeter(Perimeter per, float* source);
    void setSelection(std::vector<std::vector<int> > perims);
    
    // later add some reasonable functions from which we can actually make some sense.. 

    private :
	void setPerimeterParameters(Perimeter& per, char* perimeterMask, int o_x, int o_y, int w, int h);  // passed by reference so we just have to modify it.. 

    int* refCounter;
    void initialize(float* source);
    bool mergeMasks(Perimeter per, float* source);
    std::vector<int> tracePerimeter(char* m, int ox, int oy, int w, int h, int gw, char out);

    std::vector<Perimeter> perimeters;
    Perimeter outlinePerimeter;
    std::vector<Perimeter> selectedPerimeters;  // set by the user.. 
    char* mask;
    int mask_ox, mask_oy, mask_w, mask_h;
    float* values;
    char bvalue, outvalue;
    
};



#endif

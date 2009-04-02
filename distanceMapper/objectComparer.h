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

#ifndef OBJECTCOMPARER_H
#define OBJECTCOMPARER_H

#include "../dataStructs.h"

class ObjectComparer {
    public :
	ObjectComparer(){
	o_data = t_data = d_data = 0;
    }
    ~ObjectComparer(){
//	delete o_data;  // this is questionable.. 
	delete t_data;
	delete d_data;
    }
    
    void setData(float* data, int objects, int dimensions, float sigma=0.0, float power=0.0);  //if sigma is 0.0 do normal
    objectDistanceInfo distances();
    // and some access functions so that we can actually use the thingy.. 

    private :
	float* o_data;
    float* t_data;
    float* d_data;   // these are the original data, the transformed data and the distance data..
    int dimNo, objNo;
    float sig, order;
    bool isFlat;
    // some internal functions
    void normalise();  // applies some normalisation to each parameter
    void flatten();    // flattens using a sigmoidal curve if specificed
    void compare();    // does the comparison and creates the distance matrix..

};

#endif

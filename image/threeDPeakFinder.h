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

#ifndef THREEDPEAKFINDER_H
#define THREEDPEAKFINDER_H

#include "../dataStructs.h"
#include <vector>

class ThreeDPeakFinder {
    public :
	ThreeDPeakFinder(){
	data = 0;
	mask = 0;
	radius = 0;
    }
    ~ThreeDPeakFinder();
    
    std::vector<simple_drop> findPeaks(float* voxels, int w, int h, int d, int xb, int yb, int zb, float mpv, float mep, int r, 
				       int wavelength, int waveIndex,
				       int x_margin, int y_margin, int z_margin);

    private :
	bool checkShellLayer(int x, int y, int z, int r, float pv, float& maxValue);
    bool checkNeighbour(int dx, int dy, int dz, int x, int y, int z, float mev);

    int radius;
    int diameter;
    int area, volume;
    float* data;
    char* mask;
    int width, height, depth;
    int xbegin, xend, ybegin, yend, zbegin, zend;
    float minPeakValue, maxEdgeProportion;

};

#endif

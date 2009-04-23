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

#include "threeDPeakFinder.h"
#include <stdlib.h>

using namespace std;

ThreeDPeakFinder::~ThreeDPeakFinder(){
    delete mask;  // this is supposed to be safe in C++ 
}

vector<simple_drop> ThreeDPeakFinder::findPeaks(float* voxels, int w, int h, int d, int xb, int yb, int zb, float mpv, float mep, int r, 
						int wavelength, int waveIndex,
						int x_margin, int y_margin, int z_margin)
{
    data = voxels;
    width = w; height = h; depth = d;
    xbegin = xb; ybegin = yb; zbegin = zb;
    minPeakValue = mpv;
    maxEdgeProportion = mep;
    

    if(radius != r && mask){
	delete mask;
	mask = 0;
    }
    radius = r;
    diameter = r * 2 + 1;
    area = diameter * diameter;
    volume = diameter * diameter * diameter;
    if(!mask){
	mask = new char[volume];
    }
//    cout << "ThreeDPeakFinder::findPeaks completed basic init" << endl;
    // and then do stuff.. 
    vector<simple_drop> drops;
    drops.resize(0);
    drops.reserve(100);
    // go through points, if a point is more than mep, memset the mask to 0, then
    // go through the layers, making some sort of reasonable decision on how that goes.
    for(int z=z_margin; z < (depth - z_margin); ++z){
//	cout << "Going through layer " << z << endl;
	for(int y=y_margin; y < (height - y_margin); ++y){
//	    cout << "\t" << y << endl;
	    for(int x=x_margin; x < (width - x_margin); ++x){
		if(data[z * width * height + y * width + x] >= minPeakValue){
		    //	    cout << "+" << endl;
		    memset((void*)mask, 0, volume);
		    mask[r * area + r * diameter + r] = 1;
		    //  cout << "|" << endl;
		    bool reject = false;
		    float pv = data[z * width * height + y * width + x];
		    // cout << pv << "|" << endl;
		    float maxValue = 0;  // in fact this is the max for a given shell layer.. 
		    for(int i=1; i <= radius; ++i){
			//cout << "-" << i << "-" << endl;
			if(!checkShellLayer(x, y, z, i, pv, maxValue)){
			    reject = true;
			    break;
			}
		    }
//		    cout << endl << "checked shell layers and reject is " << reject << endl;
		    if(!reject && maxValue <= pv * maxEdgeProportion){
			// then the drop has been accepted and we are happy. Make some kind of data structure
			// that we can remember this happy occurence by..
			vector<float> pvalues(volume, 0);
//			float peakValue = data[z * width * height + y * width + x];
			int xb, xe, yb, ye, zb, ze;
			xb = xe = x;
			yb = ye = y;
			zb = ze = z;
//			cout << " and am ready to fill in the values in the peak volume" << endl;
			for(int dz=-r; dz <= r; ++dz){
			    for(int dy=-r; dy <= r; ++dy){
				for(int dx=-r; dx <= r; ++dx){
//				    cout << (int)mask[(dz + r) * area + (dy + r) * diameter + dx + r] << "  ";
				    if(mask[(dz + r) * area + (dy + r) * diameter + dx + r]){
					pvalues[(dz + r) * area + (dy + r) * diameter + dx + r] = data[(z + dz) * width * height + (y + dy) * width + x + dx];
					xb = xb > x + dx ? x + dx : xb;
					yb = yb > y + dy ? y + dy : yb;
					zb = zb > z + dz ? z + dz : zb;
					xe = xe < x + dx ? x + dx : xe;
					ye = ye < y + dy ? y + dy : ye;
					ze = ze < z + dz ? z + dz : ze;
				    }
				}
			    }
			}
//			cout << endl;
//			cout << "and was quite able to do that " << endl;
			drops.push_back(simple_drop(radius, x + xbegin, y + ybegin, z + zbegin, 
						    xb + xbegin, xe + xbegin, 
						    yb + ybegin, ye + ybegin, 
						    zb + zbegin, ze + zbegin, 
						    pvalues, waveIndex, wavelength));
		    }
		}
	    }
	}
    }
//    cout << "found some drops " << drops.size() << endl;
    return(drops);
}


bool ThreeDPeakFinder::checkShellLayer(int x, int y, int z, int r, float pv, float& maxValue){
    // x, y and z refer to the core. We essentially have to check things in a reasonable manner..
    int zp, yp, xp;
    maxValue = 0;
//    cout << "checkShellLayer " << x << ", " << y << ", " << z << "  r: " << r << " peak Value " << pv << "  maxValue : " << maxValue << endl;
    for(int dz=-r; dz <= r; ++dz){
	zp = z + dz;
	if(zp < 0 || zp >= depth)
	    continue;
	for(int dy=-r; dy <= r; ++dy){
	    yp = y + dy;
	    if(yp < 0 || yp >= height)
		continue;
	    int xinc = (abs(dz) == r || abs(dy) == r) ? 1 : 2 * r;
	    for(int dx=-r; dx <= r; dx += xinc){
		xp = x + dx;
		if(xp < 0 || xp >= width)
		    continue;
//		cout << "maxValue of shell is now " << maxValue << " and I'm about to call checkNeighbour" << endl;
		/// now.. make sure that the cell closer to the center has a higher value.. 
		if(!checkNeighbour(dx, dy, dz, x, y, z, pv * maxEdgeProportion))
		    return(false);
		int cp = zp * area + yp * diameter + xp;
		if(data[cp] > maxValue)
		    maxValue = data[cp];
	    }
	}
    }
    return(true);
}
 
bool ThreeDPeakFinder::checkNeighbour(int dx, int dy, int dz, int x, int y, int z, float mev){
    int zd = dz ? dz / abs(dz) : 0;  
    int yd = dy ? dy / abs(dy) : 0;
    int xd = dx ? dx / abs(dx) : 0; // becauuse this gives us -1, 0 or 1

//    cout << "checkNeighbour " << dx << ", " << dy << ", " << dz << "  : " << xd << ", " << yd << ", " << zd << endl;

    // there are three types of circumstances for each dimension..
    // 1. we are on the edge of the dimension
    // 2. we are one in from the edge
    // 3. we are more than one in from the edge

    // Alternatively we can be more simple and say that we are just going to check one position
    // for each one. And that position is the one that points towards the center.
    // Note that we need some way of checking if a position is a legal one or not to allow for 
    // some kind of assymetry in the shape of dots (which will tend to happen if the dot is close to another
    // dot: in that case we need to stop it on one side, but not the other.. 
    int xp = x + dx;
    int yp = y + dy;
    int zp = z + dz;
    
    int cp = zp * width * height + yp * width + xp;
    int pp = (zp - zd) * width * height + (yp - yd) * width + xp - xd; // the previous position against which we check.
    int m_cp = (dz + radius) * diameter * diameter + (dy + radius) * diameter + dx + radius;
    int m_pp = (dz + radius - zd) * diameter * diameter + (dy + radius - yd) * diameter + dx + radius - xd;

//     if((x + xbegin) == 646 && (y + ybegin) == 600 && (z + zbegin) == 23){
// 	cout << "\t" << x << "," << y << "," << z << " | " << xp << ", " << yp << ", " << zp << " | " << xp - xd << ", " << yp - yd << ", " << zp - zd << " | "
// 	     << " : " << dx << ", " << dy << ", " << dz << " : " 
// 	     << xd << ", " << yd << ", " << zd << "  cp : " << zp * width * height + yp * width + xp
// 	     << "  pp : " << (zp - dz) * width * height + (yp - dy) * width + xp - xd << "  (" << xp-dx << ", " << yp-dy << ", " << zp-dz << ")" 
// 	     << "  mask[pp] : " << (int)mask[m_pp] << "  value pp " << data[pp] << "  value cp " << data[cp] << endl;
//     }

    // The following condition seems a little strange... but unless we can make a valid comparison, we'll just keep going.
    // since returning false will mean that we give up on this peak, the criteria for returning false must be quite strict.. 
    if(!mask[m_pp])
	return(true);
    
    if(data[cp] > data[pp] && data[pp] > mev)
	return(false);
    
    if(data[cp] > data[pp])
	return(true);

    mask[m_cp] = 1;

    return(true);
}
       



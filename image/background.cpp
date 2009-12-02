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

#include "background.h"
#include <vector>
#include <algorithm>

using namespace std;

Background::Background(ImageData* id, unsigned int xm, unsigned int ym, unsigned int zm, float pcnt){
    data = id;
    x_m = xm; y_m = ym; z_m = zm;
    wi = -1;
    percentile = (pcnt < 100 && pcnt >= 0) ? pcnt : 5;
    background = 0;
    data->dims(width, height, depth);
}

Background::Background(ImageData* id, backgroundPars bgp){
  data = id;
  x_m = bgp.x_m; y_m = bgp.y_m; z_m = bgp.z_m;
  percentile = bgp.pcntile;
  background = 0;
  data->dims(width, height, depth);
}

Background::~Background(){
    delete background;
}

void Background::setBackground(unsigned int waveindex){
    if((int)waveindex == wi)
	return;
    wi = waveindex;
    setBackground();
}

float Background::bg(int x, int y, int z){
    // use a linear interpolation to get an estimate for the given position.
    if(x >= width || y >= height || z >= depth){
	cerr << "Background::bg point outsde area: " << x << "," << y << "," << z << endl;
	return(0);
    }
    // this is why I don't like unsigned ints.
    int xb = ((int)x - (int)x_m/2) / (int)x_m;
    int yb = ((int)y - (int)y_m/2) / (int)y_m;
    int zb = ((int)z - (int)z_m/2) / (int)z_m;
    
    // if x cannot be negative, then, x - x_m/2 >= -x_m/2
    // this means that the smallest number will be -0.5, which
    // will round to 0.
    
    // Note that if it is 0, it doesn't actually matter, we will then do an interpolation
    // with two points which are both on the same side of the point. That is perfectly ok.
    // 
    // However, if xb is the last value (i.e. xb + 1) goes out of bounds, then we have a problem
    // as x_b goes out of bounds. Still, we can use the same logic and just reset it to be no larger
    // than bw - 2.
    
    xb = xb < (int)bw-1 ? xb : bw-2;
    yb = yb < (int)bh-1 ? yb : bh-2;
    zb = zb < (int)bd-1 ? zb : bd-2;

    // there are four edges that can be compressed in the x dimension.
    // Compress these and obtain values for each one of them
    vector<float> xc(4);
    for(uint dz=0; dz < 2; ++dz){
	for(uint dy=0; dy < 2; ++dy){
	    uint pl = b_off(xb, yb + dy, zb + dz);
	    uint pr = b_off(xb+1, yb + dy, zb + dz);
	    xc[dz * 2 + dy] = background[pl] + (float(x - bg_pos[pl].x) / float(x_m)) * (background[pr] - background[pl]);
	}
    }
    // We now have four values in the same x-plane, but different y and z positions.
    // First compress these in the y plane.
    uint bp = b_off(xb, yb, zb);
    float front = xc[0] + (float(y - bg_pos[bp].y)/float(y_m)) * (xc[1] - xc[2]);
    float back = xc[2] + (float(y - bg_pos[bp].y)/float(y_m)) * (xc[3] - xc[2]);

    // and then simply interpolate in the z direction..
    float int_back = front + (float(z - bg_pos[bp].z) / float(z_m)) * (back - front);
//     if(int_back > 1){
//       cout << "int_back: " << int_back << " pos:  " << x << "," << y << "," << z
// 	   << "  b pos : " << xb << "," << yb << "," << zb << "  bg: " << background[ b_off(xb, yb, zb)] 
// 	   << "  bg_pos[bp]: " << bg_pos[bp].x << "," << bg_pos[bp].y << "," << bg_pos[bp].z << "\n"
// 	   << "xc : " << xc[0] << "," << xc[1] << "," << xc[2] << "," << xc[3]
// 	   << "  front: " << front << "  back: " << back << endl;
//       cout << "\t" << front << " + " <<  (float(z - bg_pos[bp].z) / float(z_m)) << " * " <<  (back - front)
// 	   << "  (z_m) : " << z_m << endl;
//     }
    // cout << "int_back : " << int_back << endl;
    return(int_back);
}

void Background::setBackground(){
    // The number of elements required will be
  cout << "Background::setBackground " << "waveIndex is : " << wi << "  pars : "
       << x_m << "," << y_m << "," << z_m << "  pcntile" << percentile << endl;
    bw = width / x_m; 
    bh = height / y_m; 
    bd = depth / z_m;
    b_length = bd * bh * bw;
    cout << bw << "x" << bh << "x" << bd << " --> " << b_length << endl;
    if(!b_length){
	cerr << "Background::setBackground 0 b_length dims : " 
	     << depth << "/" << z_m
	     << " , " << height << "/" << y_m 
	     << " , " << width << "/" << x_m << endl;
	return;
    }
    if(background)
	delete background;
    background = new float[b_length];
    bg_pos.resize(b_length);
    // then loop around and find a useful thingy. 
    for(uint bz=0; bz < bd; ++bz){
	for(uint by=0; by < bh; ++by){
	    for(uint bx=0; bx < bw; ++bx){
		background[ bz * (bh * bw) + by * bw + bx ] = getb(bx, by, bz);
		bg_pos[bz * (bh * bw) + by * bw + bx ] = pos( ((bx+1) * x_m ) - x_m/2,
							      ((by+1) * y_m ) - y_m/2,
							      ((bz+1) * z_m ) - z_m/2 );
	    }
	}
    }
    cout << "background Set" << endl;
}

void Background::setParameters(int xw, int yw, int zw, float pcnt){
    x_m = xw;
    y_m = yw;
    z_m = zw;
    percentile = pcnt;
    setBackground(wi);
}

void Background::setParameters(backgroundPars bgp){
  setParameters(bgp.x_m, bgp.y_m, bgp.z_m, bgp.pcntile);
}

float Background::getb(int bx, int by, int bz){
    int z_b = bz * z_m;
    int z_e = z_b + z_m < depth ? z_b + z_m : depth;

    int y_b = by * y_m;
    int y_e = y_b + y_m < height ? y_b + y_m : height;

    int x_b = bx * x_m;
    int x_e = x_b + x_m < width ? x_b + x_m : width;

    int vl = (x_e - x_b) * (y_e - y_b) * (z_e - z_b);
//     cout << "getb x: " 
// 	 << bx << " : " << x_b << "->" << x_e 
// 	 << "  y: " << by << " : " << y_b << "->" << y_e
// 	 << "  z: " << bz << " : " << z_b << "->" << z_e << endl; 
    if(!vl){
	cerr << "Background::getb, empty cube specified, returning 0";
	return(0);
    }
    vector<float> v;
    v.reserve(vl);
    float dp;
    for(int z=z_b; z < z_e; ++z){
	for(int y=y_b; y < y_e; ++y){
	    for(int x=x_b; x < x_e; ++x){
		data->point( dp, x, y, z, (uint)wi );
		v.push_back(dp);
	    }
	}
    }
    sort(v.begin(), v.end());
    return( v[ (uint)((float)vl * percentile / 100.0) ] );
}



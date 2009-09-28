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

/*
  Approximates backgrounds by dividing up the volume into cubes and taking the n-th
  percentile value as the background for the volume. Estimates backgrounds for
  individual points by taking averages weighted by distance from neighbouring points
  (such that the individual weights add up to 1).
  This isn't really ideal, but the ideal solutions will take too much processing and / or
  memory.
*/


#include "imageData.h"
#include <vector>

// The Background class does not take ownership of the ImageData pointer. And will not delete
// it in it's destructor. A reference to an object might be preferrable, but, user beware is
// easier to implement.

struct pos {
    int x, y, z;
    pos(){
	x=y=z=0;
    }
    pos(int X, int Y, int Z){
	x=X; y=Y; z=Z;
    }
};

class Background {
    public :
	Background(ImageData* id, unsigned int xm, unsigned int ym, unsigned int zm, float pcnt=5);
    ~Background();
    void setBackground(unsigned int waveindex);
    float bg(int x, int y, int z);
    
 private:
    ImageData* data;
    int wi;
    float percentile;
    unsigned int x_m, y_m, z_m; // the multipliers (size of fields).
    float* background;
    std::vector<pos> bg_pos;    // the positions represented by the background values (unnecessary, but easier)
    unsigned int width, height, depth;  // width height and depth of the image Data
    unsigned int bw, bh, bd;            // the background dimensions.
    unsigned int b_length;              // the number of background things calculated

    void setBackground();
    float getb(int bx, int by, int bz);
    
    uint b_off(int x, int y, int z){
	return( z * bh * bw + y * bw + x );
    }
    
};



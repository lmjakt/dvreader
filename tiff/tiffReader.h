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

#ifndef TIFFREADER_H
#define TIFFREADER_H

// a basic class that reads a tiff file ?? into an array of
// ints. Well, not really, it reads a one bit tiff file produces
// by our microscope thingy..

// This is a dirty simple and a bit stupid thing. Use carefully, does not suppport very much..

#include <vector>
#include <string>

typedef unsigned int uint;

using namespace std;

class TiffReader {
  public :
    TiffReader(string iFile);      // create from a file
  TiffReader();                    // just instantiate..
  TiffReader(TiffReader* tr);      // just copy out the values from this one,, 
  bool readFile(string iFile);     // read the values from file. returns true on success
  bool makeFromRGBFloat(float* data, uint wdth, uint hgt);  // assume RGB values between 0 and 1. 
  bool makeFromRGBA(unsigned char* data, uint width, uint hgt);  // throws away the alpha channel at the moment..
  bool addMerge(TiffReader* tr);      //does an additive merge.. just adds up the values, .. -keeping below 255 
  vector< vector<unsigned short> > data();
  vector< vector<short> > colours();        // return the data and the colour map. 
  vector< vector<unsigned short> > values;     // 2 bytes per value,, hmm, chutto wasteful  cast for other purposes.. 
  vector< vector<unsigned short> > colorMap;  // if the input file has a colour map.. 
  bool indexed;                               // true if indexed with colour map, 
  string fileName;
  int width();
  int height();
  void writeOut(string outFile);

 private: 
  vector<unsigned short> bitsPerSample;                // 
  unsigned short bytesPerSample;
  /////////////////////  if the map is indexed, then the index size might be more than 256, hence use the short.
  unsigned int w;
  unsigned int h;
  unsigned short compressionScheme;
  unsigned short photometricInterpretation;
};

#endif

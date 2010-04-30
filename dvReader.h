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

#ifndef DVREADER_H
#define DVREADER_H

#include <string.h>    // for memcopy
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
//#include "imageData.h"
#include "dataStructs.h"
#include "kcluster/kClusterProcess.h"
#include "panels/fileSet.h"




//// UPDATE TO ARBRITRARY IMAGE FORMAT READING BY SPECIFYING DIFFERENT FILE FORMATS AND 
//// HAVE DIFFERENT PARSING FUNCTIONS.. RATHER THAN IN THE CONSTRUCTORE.
//// CHANGE NAMES EVENTUALLY TO REFLECT THIS.

// DVReader. initially, load one image at a time.. just for going through the stack..
// Later, I will probably set this up to load all the data into one huge array of data..

// default behaviour now, is to read in the header information. Ignore the contents of the
// extended header, as I'm not sure what's in there, though we can have a poke around a bit and work it out.
// note that I'm using a void pointer, as I can't know beforehand how the data will be stored. It will
// probably be in 2bit ints (shorts) but I can't be certain of that. Note that for a variety of purposes
// I may want to maintain a second data set with the data mapped to 256 levels of gray, as this is all that the
// display outupts. (This mapping will have to take into account min and max, but it may be that this 
// should really be provided by an alternative mechanism..)


/// objects are made up of points which are specifed by position wavelength and intensity.
/// note that this is horribly memory intensive if lots of pixels will participate as points.
/// However, hopefully that won't be the case, and 4 Gb will hopefully be enough.. (or we will have to
/// go to disk. Which we don't want to do...



class DVReader
{
    
    public :
    DVReader(const char* fName, float maxlevel);
  ~DVReader();
  // then public functions..
  enum FileFormat { 
    DELTAVISION, ICS_V1, ICS_V2
  };
  int nextSection();   // load next image.. (ignore which wavelength this is.)
  int previousSection();
  void setBiasAndScale(std::vector<float> bias, std::vector<float> scale);
//  void setAdditive(std::vector<bool> addFactors);
  void setImageData();              // set the image data for the current section (perhaps using new biases);
  void setWaveColor(int wave, float r, float g, float b);  // set colors for channel with wavelength wave.. 
  void setOffsets(int wave, int xo, int yo, int zo){
      //  image->setOffsets(wave, xo, yo, zo);
  }
  void useComponentFactors(bool on);
  void addMergeChannel(int* mergeI, int s){
      //image->addMergeChannel(mergeI, s);
  }
  float* rgb_image(){ 
    return(currentRGBImage);   // which are the chosen colours..  */
  } 
  float* frameData(){
      return(0);
//    return(image->frameData(currentSection));
  }
  float* x_zSlice(int ypos);
  //float* y_zSlice(int xpos);
  //  unsigned char* image(){
  //    return(displayData);
  // }
  short channelNo(){
//    return(image->channelNo());
      return(0);
    //return(nw + mergedWaves);
  }
  int* channels(){
    return(waves);
  }
  short channel(int n){
    if(n < nw){
      return(waves[n]);
    }
  }

  int sliceSize(){
    return(sectionSize);
  }
  int sliceNo(){
    return(secNo);
  }
  int width(){
    return(nx);
  }
  int height(){
    return(ny);
  }
  int bits(){
    return(pSize * 8);
  }

  
  int currentSectionNo(){
      return(currentSection);
  }

  void printInfo();   // prints info
  // void findObjects(int wl, float minValue, float minFraction, float minIncludeFraction, int boxSize);    // find objects.. 
  void clearObjects();
  void dimensions(float& w, float& h, float& d){   // the width, height and depth of the stack in umeters..
    w = (float)nx * dx;
    h = (float)ny * dy;
    d = (float)secNo * dz;
  }
  std::vector<float> objectSums(int wl);   // wl the wavelength.. 


  FileSet* fileData(){   // this can then be handled directly by deltaviewer.. 
      return(fileSet);
  }

  private :
//    std::ifstream scratch;           // point to a scratch file containing the data.. 
//  std::string scratchFile;
    float* currentRGBImage;      // the current image..
//  imageData* image;            // contains the data in float format and handles things like getting data in and out..
  FileSet* fileSet;            // accesses the file or files containing the data and interprets these in a reasonable manner ?
  float maxLevel;
  //  groupObject** objects;         // one object for each pixel. Don't seperate for individual wavelengths yet. Not enough memory
  std::ios::pos_type headerSize;   // this is usually 1024, but might be different I suppose.. 
  std::ios::pos_type sectionSize;
  int currentSection;
  short pSize;   // the number of bytes for each pixel.. 
  int margin;  // the deconvolution process includes a margin at the edges. we want to ignore pixels in this area
  int nx, ny, nsec, mode, mxst, myst, mzst, mx, my, mz;
  int secNo;     // the actual number of sections.. 
  float dx, dy, dz, alpha, beta, gamma;
  int xaxis, yaxis, zaxis;
  float min, max, mean;
  short type, nspg;
  int next;
  short dvid;
  char blank[30];
  short nint;
  short nreal;
  short sub;
  short zfac;
  float min2, max2, min3, max3, min4, max4;
  short type_2, lensid, n1, n2, v1, v2;
  float min5, max5;
  short nt, interleaving;
  float xTilt, yTilt, zTilt;
  short nw, wave1, wave2, wave3, wave4, wave5;
  int waves[5];   // maximum 5..so far  
  float mins[5];
  float maxs[5];  // one for each wave.. 
  float waveColors[5][3];   // the color chosen for that wave... i.e. how it gets blended into the final picture..  // initialise to 0..  
  float x0, y0, z0;
  int nttl;
  // then there are ten optional titles I don't think I can really be bothered to deal with them
  // right now.
  // void setDisplayData(int wno);  // provide the channel number so we now which max or min to use.. 
  //void setCurrentLimits(int wno);
  bool readDataFromFile(const char* fName, FileFormat format);
  bool readDVFile(const char* fName);
  bool readICSv1File(const char* fName);
  
  bool sameEndian;    // try to guess if the file has the same endiannes as the system.. 

  std::vector<std::string> getWords(char ws, char ls, std::ifstream& in);
  bool readBytes(int bn, float* row, std::ifstream& in);   // read n bytes from current position in file. 
  // function for finding neighbours of a given point.. that have at least some fraction of the intensity of the given point
  //  void findNeighbours(point* currentPoint, point* fromPoint, groupObject* gObject, float minFraction); 
  
  // fast inline function for swapping bytes in data.... 
  void swapBytes(char* from, char* to, unsigned int wn, unsigned int ws){
      for(uint i=0; i < wn; i++){        // wn is the number of words, hence the byte length is ws * wn ..
	  for(uint j=0; j < ws; j++){
	      /// assign first byte of to as the last value of the ith word of from
	      to[i * ws + j] = from[(i+1) * ws - (j + 1)];
	  }
      }
  }
  void swapBytes(char* data, unsigned int wn, unsigned int ws);
  void readShort(std::ifstream& in, short& n){
      in.read((char*)&n, 2);   // though sizeof(short) might be more appropriate..
      if(!sameEndian){
	  short swapped;
	  swapBytes((char*)&n, (char*)&swapped, 1, 2);
	  n = swapped;
      }
  }
  void readInt(std::ifstream& in, int& n){
      in.read((char*)&n, 4);
      if(!sameEndian){
	  int swapped;
	  swapBytes((char*)&n, (char*)&swapped, 1, 4);
	  n = swapped;
      }
  }
  void readFloat(std::ifstream& in, float& f){
      in.read((char*)&f, 4);
      if(!sameEndian){
	  float swapped;
	  swapBytes((char*)&f, (char*)&swapped, 1, 4);
	  f = swapped;
      }
  }
  
};


#endif

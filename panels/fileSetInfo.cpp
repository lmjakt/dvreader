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

#include "fileSetInfo.h"
#include <iostream>
#include <fstream>

using namespace std;

void readInt(ifstream& in, int& value){
    in.read((char*)&value, sizeof(int));
}

void readInt(ifstream& in, unsigned int& value){
    in.read((char*)&value, sizeof(int));
}

void readFloat(ifstream& in, float& value){
    in.read((char*)&value, sizeof(float));
}

void writeFloat(ofstream& out, float value){
    out.write((char*)&value, sizeof(float));
}

void writeInt(ofstream& out, int value){
    out.write((char*)&value, sizeof(int));
}

void writeInt(ofstream& out, unsigned int value){
    out.write((char*)&value, sizeof(int));
}

FileSetInfo::FileSetInfo(const char* fname){
    stackNo = 0;
    fileName = fname;
    cout << "FileSetInfo::FileSetInfo reading infrom file : " << fname << endl;
    ok = true;
    ifstream in(fname);
    if(!in){
	cout << "Unable to read file info from file so returning with ok set to false" << endl;
	ok = false;
	return;
    }
    int magic;
    in.read((char*)&magic, sizeof(int));
    if(magic != magicNumber){
	cerr << "FileSetInfo magicNumber is not " << magicNumber << " but : " << magic << endl;
	ok = false;
	return;
    }
    // then .. 
    readInt(in, stackNo);
    readInt(in, waveNo);
    readInt(in, width);
    readInt(in, height);
    
    cout << "magic : " << magic << endl;
    cout << "\tstackNo " << stackNo << "\n"
	 << "\twaveNo " << waveNo << "\n"
	 << "\twidth x height " << width << " x " << height << endl;

    if(width * height > (uint)maxPixelNo){
	cerr << "dimensions of stacks are larger than max allowed size : " << width << " x " << height << endl;
	ok = false;
	return;
    }

    if(stackNo < 0 || stackNo > maxStackNumber){
	cerr << "FileSetInfo::FileSetInfo stackNo " << stackNo << "  is inappropriate maxStackNumber is set to : " << maxStackNumber << endl;
	ok = false;
	return;
    }
    // and then just read in each wavelength
    waveLengths.resize(waveNo);
    for(uint i=0; i < waveLengths.size(); ++i){
	readInt(in, waveLengths[i]);
	cout << "read wavelength : " << waveLengths[i] << endl;
    }
    
    // then read
    // for each thingy..
    for(int i=0; i < stackNo; ++i){
	// first read in the floats for the map positions..
	float mx, my;
	readFloat(in, mx);
	readFloat(in, my);
	FrameInfo* finfo = new FrameInfo();
	readFloat(in, finfo->xpos);
	readFloat(in, finfo->ypos);
	readInt(in, finfo->xp);
	readInt(in, finfo->yp);
	readInt(in, finfo->width);
	readInt(in, finfo->height);
	readInt(in, finfo->waveNo);   // this is merely to check it against the wave number given above..
	cout << i << "\tRead in frameinfo at position : " << finfo->xp << ", " << finfo->yp << endl;
	// although this is an unsigned int.. hmm. 
	if(finfo->waveNo != waveLengths.size()){
	    cerr << "FileSetInfo::FileSetInfo waveNo != waveLengths.size() " << waveNo << " : " << waveLengths.size() << endl;
	    cerr << "WARNING " << fname <<  " is probably corrupted" << endl;
	    cerr << "WARNING : THIS WILL RESULT IN A MAJOR MEMORY LEAK UNTIL I SORT OUT THIS FUNCTION TO CLEAN UP AFTER ITSELF" << endl;
	    ok = false;
	    return;
	}
	// I should perhaps also do a check on the width and length as well as checking on the state of he
	// ifstream, but really can't be bothered.. 

	finfo->projection = new float*[waveLengths.size()];
	for(uint j=0; j < waveLengths.size(); j++){
	    finfo->projection[j] = new float[width * height];
	    in.read((char*)finfo->projection[j], sizeof(float) * width * height);
	}
	stacks[mx][my] = finfo;
    }
    
    ok = true;
}

FrameInfo* FileSetInfo::getStack(float x, float y){
    if(!stacks.count(x)){
	return(0);
    }
    if(!stacks[x].count(y)){
	return(0);
    }
    return(stacks[x][y]);
}

void FileSetInfo::addFrameInfo(FrameInfo* finfo, float x, float y){
    stacks[x][y] = finfo;
    stackNo++;
}

bool FileSetInfo::writeInfo(const char* fname){
  fileName = fname;
  ofstream out(fname);
  if(!out){
    cout << "FileSetInfo::writeInfo unable to open " << fname << "  for writing " << endl;
    return(false);
  }
  // and then just write in the same order as we read..
  writeInt(out, magicNumber);
  writeInt(out, stackNo);
  //    writeInt(out, (int)stacks.size());
  writeInt(out, waveNo);
  writeInt(out, width);
  writeInt(out, height);
  for(uint i=0; i < waveLengths.size(); i++){
    writeInt(out, waveLengths[i]);
  }
  map<float, map<float, FrameInfo*> >::iterator ot;
  map<float, FrameInfo*>::iterator it;  // outer and inner iterators..
  for(ot = stacks.begin(); ot != stacks.end(); ot++){
    for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
      FrameInfo* finfo = (*it).second;
      writeFloat(out, (*ot).first);
      writeFloat(out, (*it).first);
      writeFloat(out, finfo->xpos);
      writeFloat(out, finfo->ypos);
      writeInt(out, finfo->xp);
      writeInt(out, finfo->yp);
      writeInt(out, finfo->width);
      writeInt(out, finfo->height);
      writeInt(out, finfo->waveNo);
      cout << "Wrote out finfo with position " << finfo->xp << ", " << finfo->yp << endl;
      for(uint i=0; i < waveNo; i++){
	out.write((char*)finfo->projection[i], finfo->width * finfo->height * sizeof(float));
      }
    }
  }
  return(true);  // well I should check the state of the stream here, but don't remember how, and not sure what to do about it anyway.. 
}

bool FileSetInfo::writeInfo(){
  return( writeInfo( fileName.c_str() ) );
}

// the total width of the arrangement
void FileSetInfo::dims(int& w, int& h)
{
  int max_x = 0;
  int max_y = 0;
  int min_x = 0;
  int min_y = 0;
  bool initial = true;
  for(map<float, map<float, FrameInfo*> >::iterator ot=stacks.begin();
      ot != stacks.end(); ++ot){
    for(map<float, FrameInfo*>::iterator it=(*ot).second.begin();
	it != (*ot).second.end(); ++it){
      if(initial){
	min_x = (*it).second->xp;
	max_x = (*it).second->xp;
	min_y = (*it).second->yp;
	max_y = (*it).second->yp;
	initial = false;
	continue;
      }
      min_x = min_x > (*it).second->xp ? (*it).second->xp : min_x;
      max_x = max_x < (*it).second->xp ? (*it).second->xp : max_x;
      min_y = min_y > (*it).second->yp ? (*it).second->yp : min_y;
      max_y = max_y < (*it).second->yp ? (*it).second->yp : max_y;
    }
  }
  w = (max_x + width) - min_x;
  h = (max_y + height) - min_y;
}

int FileSetInfo::image_width(){
  int w, h;
  dims(w, h);
  return(w);
}

int FileSetInfo::image_height(){
  int w, h;
  dims(w, h);
  return(w);
}

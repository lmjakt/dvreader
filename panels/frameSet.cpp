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

#include "frameSet.h"
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

FrameSet::FrameSet(int* waveLengths, int waveNo){
//    for(int i=0; i < waveNo; i++){
//	waves.insert(waveLengths[i]);
//    }
    pWidth = pHeight = 0;
    x = y = z = w = h = 0;
}

FrameSet::~FrameSet(){
    while(frames.size()){
	delete (*frames.begin()).second;
	frames.erase(frames.begin());
    }
}

bool FrameSet::addFrame(const char* fname, std::ios::pos_type framePos, std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
			short numInt, short numFloat, unsigned short byteSize,
			bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz){
    // first make a frame.
    // then if this is ok, then go ahead and set the parameters and so forth..
    
    // first open the file..
    ifstream* in = new ifstream(fname);
    if(!(*in)){
	cerr << "FrameSet addFrame unable to open " << fname << " for reading" << endl;
	delete in;
	return(false);
    }
    
    Frame* frame = new Frame(in, framePos, readPos, extHeadSize, numInt, numFloat, byteSize, real, bigEnd, width, height, dx, dy, dz);
    return(addFrame(frame));
}
    
bool FrameSet::addFrame(Frame* frame){
    if(!frame->ok()){
	cerr << "FrameSet::addFrame Frame reporting error in construction returning false" << endl;
	delete frame;
	return(false);
    }
    // check if the wavelength of the frame is acceptable..
//    if(!waves.count(frame->emission())){
//	cerr << "FrameSet::addFrame Frame reports an unknown emission wavelength : " << frame->emission() << endl;
//	delete frame;
//	return(false);
//    }
    // then check if we have some frames already..
    fluorInfo fluor(frame->excitation(), frame->emission(), frame->exposure());
    if(!frames.size()){
	// first set the appropriate parameters..
	x = frame->xPos();
	y = frame->yPos();
	z = frame->zPos();
	w = frame->sampleWidth();
	h = frame->sampleHeight();
	pWidth = frame->p_width();
	pHeight = frame->p_height();
	frames.insert(make_pair(fluor, frame));
	fInfo.push_back(fluor);
	return(true);
    }
    // If we are here then we have to make sure that the positioning is correct
    // and that we haven't already defined a frame for this wavelength..
    if(frames.count(fluor)){
	cerr << "FrameSet addFrame. Frame already defined for emission wavelength : " << frame->emission() << "  and excitation " << frame->excitation() << frame<< endl;
	delete frame;
	return(false);
    }
    // make sure that x, y and z are the same as for the other frames (it is possible that we will have to make some sort of 
    // estimate here,, but for now let's go with equality..
    // if the reported position drifts, this may be a problem in the long run though..
    if(frame->xPos() != x || frame->yPos() != y || frame->zPos() != z){
	cerr << "FrameSet addFrame. Frame appears to have a different position to the already added frames\n current pos : " <<
	    x << ", " << y << ", " << z << "  new frame : " << frame->xPos() << ", " << frame->yPos() << ", " << frame->zPos() << endl;
	delete frame;
	return(false);
    }
    // at this point things should be ok..
    frames.insert(make_pair(fluor, frame));
    fInfo.push_back(fluor);
    sort(fInfo.begin(), fInfo.end());

    return(true);
}

void FrameSet::setBackgrounds(std::map<fluorInfo, Background*> backgrounds, int zp){
  for(map<fluorInfo, Frame*>::iterator it = frames.begin();
      it != frames.end(); it++){
    if(backgrounds.count(it->first)){
      it->second->setBackground(backgrounds[it->first], zp);
    }else{
      cerr << "FrameSet::setBackgounds no background available for : "
	   << (*it).first.excitation << " --> " << (*it).first.emission << endl;
    }
  }
}

bool FrameSet::readToRGB(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height,
			 unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, float maxLevel, vector<float> bias, 
			 vector<float> scale, vector<color_map> colors, bool bg_sub, raw_data* raw){
    // actually we haven't explicitly defined how the colors and the other things will be treated, but
    //  since we have a map<float, colors> then we can sort of assume that we will use the same ones..

    if(frames.size() > bias.size() || frames.size() > scale.size() || frames.size() > colors.size()){
	cerr << "FrameSet::readToRGB unsufficent wavelength information to read to RGB" << endl;
	return(false);
    }
    unsigned int i = 0;
    bool ok = true;
    bool read;
    // the above is a bit ugly, but it makes the following a little less ugly. 
    for(map<fluorInfo, Frame*>::iterator it = frames.begin(); it != frames.end(); it++){
      if( !(colors[i].r + colors[i].g + colors[i].b) && !raw ){
	++i;
	continue;
      }
	if(!raw){
	  read = (*it).second->readToRGB(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel, bias[i], scale[i], colors[i].r, colors[i].g, colors[i].b, bg_sub);
	}else{
//	    cout << "raw channel no is " << raw->channels << "  and length is " << raw->channels << endl;
//	    cout << "calling read with raw and positions set to : " << i << "  starting from " << raw->positions[i] << endl;
	    if(read = (*it).second->readToRGB(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel, 
					      bias[i], scale[i], colors[i].r, colors[i].g, colors[i].b, bg_sub, raw->values[i] + raw->positions[i])){
//		cout << "incrementing raw positiosn with " << width * height << endl;
		raw->positions[i] += (width * height);
//		cout << "raw positions up dated  " << endl;
	    }
	}
	if(!read){
	    cerr << "FrameSet::readToRGB some error reading wave : " << i << endl;
	    ok = false;
	}
	i++;
//	cout << "i updated " << i << endl;
    }
    return(ok);   // so a partial read gives a false..
}

//bool FrameSet::readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height,
//		   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
//		   bool bg_sub, float maxLevel, unsigned int waveIndex){
bool FrameSet::readToFloat(float* dest, unsigned int source_x, unsigned int source_y, unsigned int width, unsigned int height,
			   unsigned int dest_x, unsigned int dest_y, unsigned int dest_w, 
			   float maxLevel, unsigned int waveIndex){
    if(waveIndex >= fInfo.size()){
	cerr << "FrameSet::readToFloat waveIndex is too large : " << waveIndex << endl;
	return(false);
    }
    map<fluorInfo, Frame*>::iterator it = frames.find(fInfo[waveIndex]);
    if(it == frames.end()){
	cerr << "FrameSet::readToFloat unknown wavelength : " << waveIndex << endl;
	return(false);
    }
    //    return((*it).second->readToFloat(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, bg_sub, maxLevel));
    return((*it).second->readToFloat(dest, source_x, source_y, width, height, dest_x, dest_y, dest_w, maxLevel));
}

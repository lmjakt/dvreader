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
    but WITHOUT ANY WARRANTY; without even the implied warranty ofhttp://www.dn.se/nyheter/sverige/kraftigt-askvader--37-000-blixtnedslag
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#include "fileSet.h"
#include "borderInformation.h"
#include "frameStack.h"
#include "fileSetInfo.h"
#include "frame.h"
#include "sLookup.h"
#include "../stat/stat.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include "idMap.h"
#include "../image/background.h"
#include "../image/imageData.h"
#include "../imageBuilder/imStack.h"

using namespace std;

FileSet::FileSet(int* waveLengths, int waveNo, float maxLevel, int xy_margin){
    wave_lengths = waveLengths;
    wave_no = waveNo;
    maxIntensity = maxLevel;
    waves.resize(waveNo);
    xyMargin = xy_margin > 0 ? xy_margin : 0;
    for(unsigned int i=0; i < waves.size(); i++){
	waves[i] = wave_lengths[i];
    }
    sort(waves.begin(), waves.end());

    // and set some other things.. like ..
    x = y = w = h = 0;
    frameWidth = frameHeight = 0;
    // but since we can just do a count on the frames we don't need to set anything else up.. 
    fileName = 0;
    ifstreamCounter = 0;
    framePosMap = 0;
    rolloff = 32;      // This is only used when calling the overlap adjustment function (automatic one). We should try to get rid of it.
}

FileSet::~FileSet(){
    // delete all stacks..
    while(frames.size()){
	map<float, map<float, FrameStack*> >::iterator it = frames.begin();
	while((*it).second.size()){
	    delete (*(*it).second.begin()).second;
	    (*it).second.erase((*it).second.begin());
	}
	frames.erase(it);
    }
    for(uint i=0; i < overlapData.size(); i++){
	delete overlapData[i];
    }
    delete framePosMap;
    for(map<fluorInfo, SLookup*>::iterator it=luts.begin(); it != luts.end(); ++it)
      delete (*it).second;
    luts.clear();
}

bool FileSet::addFrame(string fname, ifstream* in, std::ios::pos_type framePos, 
		       std::ios::pos_type readPos, std::ios::pos_type extHeadSize,
		       short numInt, short numFloat, unsigned short byteSize,
		       bool real, bool bigEnd, unsigned int width, unsigned int height, 
		       float dx, float dy, float dz)
{
    // make a new frame, and if its x and y coordinates don't fit then 
    // make a new framestack and stick it into that ..
  if(!fileName){
    // It seems that it is necessary that we call new to make sure that
    // the filename doesn't get overwritten.
    fileName = new char[fname.length() + 1];
    fileName[fname.length()] = 0;
    memcpy((void*)fileName, (void*)fname.c_str(), fname.length());
  }
  //if(*fileName != *fname.c_str()){
  if(string(fileName) != fname){
    cerr << "FileSet::addFrame fileName has changed, exiting due to memory corruption : " << fileName << " --> " << fname << endl;
    exit(1);
  }
  // first use a temporary ifstream as it seems we can't make and delete thousands of ifstreams..
  Frame* frame = new Frame(in, framePos, readPos, extHeadSize, 
			   numInt, numFloat, byteSize, real, bigEnd, width, height, dx, dy, dz);
  if(!frame->ok()){
    cerr << "FileSet::addFrame Frame is not ok" << endl;
    delete frame;
    return(false);
  }
  fluorInfo finfo(frame->excitation(), frame->emission(), frame->exposure());
  flInfo.insert(finfo);

  if(!luts.count(finfo))
    luts.insert(make_pair(finfo, new SLookup(1.0, 0.0, 1.0, 1.0, 1.0, 4, maxIntensity)));

  if(!photoSensors.count(finfo)){
    photoSensors[finfo] = frame->phSensor();
  }
  frame->setPhSensorStandard( photoSensors[finfo] );
  
  // and then look for a suitable framestack..
  bool stackIsNew = false;
  float x_pos = frame->xPos();
  float y_pos = frame->yPos();   // these get modified by the getStack function to point to the appropriate things for assigning the stack
  
  FrameStack* fstack = 0;
  
  if(!getStack(x_pos, y_pos)){
    stackIsNew = true;
    //    cerr << "no stack found for frame counter  "  << ifstreamCounter << endl;
    ifstream* newStream = new ifstream(fname.c_str());
    ifstreamCounter++;
    if(!(*newStream)){
      cerr << "FileSet::addFrame failed to create new filestream on new stack counter " << ifstreamCounter << endl;
      exit(1);
    }
    fstack = new FrameStack(wave_lengths, wave_no, newStream, maxIntensity, xyMargin);   // in which case we should not delete or change the ifstream around.. 
    frame_order.push_back(fstack);
  }else{
    fstack = frames[x_pos][y_pos];
  }
  // and assign the appropriate stream to the frame (the one that belongs to the filestack
  frame->setStream(fstack->fileStream());   // ugly hack, since we can't open more than about 1000 ifstreams on one file it seems.. 
  
  // then add the frame to the framestack.. 
  if(!fstack->addFrame(frame)){
    cerr << "FileSet::addFrame unable to add frame to filestack" << endl;
    delete frame;
    if(stackIsNew){
      delete fstack;
    }
    exit(1);
    return(false);
  }
  // if we are here, then we have created a new frame - we may have created a new file stack.. 
  // regardless we have to check out whether or not things match up..
  
  // 1. if this is the first one, we have to set the parameters, frameWidth, pixelHeight, .. and so on.. 
  if(!frames.size()){
    frameWidth = frame->sampleWidth();
    frameHeight = frame->sampleHeight();
    pixelHeight = frame->p_height();
    pixelWidth = frame->p_width();
    x = frame->xPos();
    y = frame->yPos();
    // and then how to insert ..
    frames[x_pos][y_pos] = fstack;
    
    //and make sure to insert the x, y and z values..
    x_set.insert(x_pos);
    y_set.insert(y_pos);
    z_set.insert(frame->zPos());
    return(true);
  }
  // first check if the coordinates are ok.. (this is not a complete check)
  if(frame->p_width() != pixelWidth || frame->p_height() != pixelHeight){
    cerr << "FileSet::addFrame frame dimensions appear to be different to previously entered values.." << endl
	 << "current : " << pixelWidth << ", " << pixelHeight << " --> " << frame->p_width() << ", " << frame->p_height() << endl;
    delete fstack;
    // do not delete frame, as this is deleted by fstack.. 
    return(false);
  }
  // we don't actually need to check if its a new stack, as it will just replace the old one in the same position
  // and then it should simply be possible to do ..
  frames[x_pos][y_pos] = fstack;   // if it already exists, then it just redefines the pointer.. 
  x_set.insert(x_pos);
  y_set.insert(y_pos);
  z_set.insert(frame->zPos());
  if(x_pos < x){ x = x_pos; }
  if(y_pos < y){ y = y_pos; }
  
  return(true);
}


fluorInfo FileSet::channelInfo(unsigned int pos){
    unsigned int i = 0;
    for(set<fluorInfo>::iterator it=flInfo.begin(); it != flInfo.end(); it++){
	if(i == pos)
	    return(*it);
	++i;
    }
    cerr << "FileSet::channelInfo() information requested for channel : " << pos << "  but only " << flInfo.size() << " channels known" << endl;
    return(fluorInfo(0, 0, 0));
}

void FileSet::stackDimensions(int& col_no, int& row_no, int& panelWidth, int& panelHeight){
  col_no = x_positions.size();
  row_no = y_positions.size();
  panelWidth = pixelWidth;
  panelHeight = pixelHeight;
}

bool FileSet::getStack(float& xpos, float& ypos){
    // since the x and y positions don't always line up. I'm simply saying if they 
    // differ by less than 0.1% then consider them to be the same. This is an arbitrary choice
    float maxDif = 0.001;
    bool foundOne = false;
    for(map<float, map<float, FrameStack*> >::iterator it = frames.begin(); it != frames.end(); it++){
	if( fabs((2 * ((*it).first - xpos)) / (xpos + (*it).first)) < maxDif ){
	    xpos = (*it).first;
	    for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
		if( fabs((2 * ((*fit).first - ypos)) / (ypos + (*fit).first)) < maxDif){
		    ypos = (*fit).first;
		    foundOne = true;
		    break;
		}
	    }
	}
	if((*it).first > xpos){
	    break;
	}
    }

    return(foundOne);
}

bool FileSet::finalise(){
    // set the vectors up. I'm not sure if I need these, but they may come in useful.. 
    x_positions.assign(x_set.begin(), x_set.end());
    y_positions.assign(y_set.begin(), y_set.end());
    z_positions.assign(z_set.begin(), z_set.end());   

    // these values are likely to be useful. 
    float xscale = frameWidth / float(pixelWidth);
    float yscale = frameHeight / float(pixelHeight);

    // to be complete, we essentially have to have a thingy at each x and y position.. and there should always be an overlap..
    // that is the simplest thing to take into account..
    completeRectangle = true;
    // it is possible that [0][0] is not defined, but we want a frame number from somewhere..
    frameNo = 0;
    for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); ++it){
      for(map<float, FrameStack*>::iterator iit=(*it).second.begin(); iit != (*it).second.end(); ++iit){
	if((*iit).second->frameNo() > frameNo)
	  frameNo = (*iit).second->frameNo();
      }
    }
    if(!frameNo){
      cerr << "FileSet::finalise framestacks appear to have no frames. aborting" << endl;
      exit(1);
    }

    //    frameNo = frames[x_positions[0]][y_positions[0]]->frameNo();
    for(uint i=0; i < x_positions.size(); i++){
	if(!frames.count(x_positions[i])){
	    cerr << "No framestacks defined for x position x: " << x_positions[i] << endl;  // though this shouldn't be possible..
	    completeRectangle = false;
	    continue;
	}
	map<float, map<float, FrameStack*> >::iterator it = frames.find(x_positions[i]);
	for(uint j=0; j < y_positions.size(); j++){
	    if(!(*it).second.count(y_positions[j])){
		cerr << "No framestacks defined for y_positions y: " << y_positions[j] << endl;
		completeRectangle = false;
	    }else{
		// set the pixelPosition of the thingy..
		int xp = int((frames[x_positions[i]][y_positions[j]]->x_pos() - x)/xscale);
		int yp = int((frames[x_positions[i]][y_positions[j]]->y_pos() - y)/yscale);
		frames[x_positions[i]][y_positions[j]]->setPixelPos(xp, yp, true);

		if(frameNo != frames[x_positions[i]][y_positions[j]]->frameNo()){
		    cerr << "Stacks have different numbers of frames. we are likely to crash at some points...." << endl;
		    if(frames[x_positions[i]][y_positions[j]]->frameNo() > frameNo){ frameNo = frames[x_positions[i]][y_positions[j]]->frameNo(); }
		}
	    }
	}
    }
    
    // This next section sets up the relationships between frames (i.e. for each frame which is it's
    // neighboring frames (left and right and top..
    if(completeRectangle){
      for(uint i=0; i < x_positions.size(); i++){
	for(uint j=0; j < y_positions.size(); j++){
	  float tx = x_positions[i];
	  float ty = y_positions[j];
	  if(i){
	    float lx = x_positions[i-1];  // left x..
	    if(!frames[tx][ty]->setNeighbour(frames[lx][ty], 0)){
	      cerr << "no overlap between " << i << "," << j << "  and  " << i-1 << "," << j << endl;
	    }
	  }
	  if(j){
	    float by = y_positions[j-1];  // below y.. 
	    if(!frames[tx][ty]->setNeighbour(frames[tx][by], 3)){   // the function is recipocral, so this is all we need to do..
	      cerr << "no overlap between " << i << "," << j << "  and  " << i << "," << j-1 << endl;
	    }
	  }
	}
      }
    }
    // first check if there is a file..

    string infoFile = fileName;
    infoFile.append(".info");

    stackInfo = new FileSetInfo(infoFile.c_str());
    if(stackInfo->ok){
      cout << "Managed to read fileSetInfo from  " << infoFile << endl;
      // Read pixel positions of files and other things and set up the 
      // projections by calling finalise() with the appropriate FrameInfo structure..
      map<float, map<float, FrameStack*> >::iterator ot;
      map<float, FrameStack*>::iterator it;
      for(ot = frames.begin(); ot != frames.end(); ot++){
	for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
	  FrameStack* fstack = (*it).second;
	  FrameInfo* finfo = stackInfo->getStack((*ot).first, (*it).first);
	  if(finfo){
	    fstack->finalise(maxIntensity, finfo);
	  }
	}
      }
      //      adjustStackBorders();
    }else{
      delete stackInfo;
      // We should do this even if the rectangle is not complete, but we have to be a bit more careful about how we set things
      // up.. -- need to put in some more things in there.. 
      
      // set up the overlaps. Only do this if the rectangle is complete, -otherwise it is a little bit too difficult..
      // make sure to make FileSetInfo and get that to write out the info so we don't have to do this again.. 
      for(uint i=0; i < x_positions.size(); i++){
	for(uint j=0; j < y_positions.size(); j++){
	  float tx = x_positions[i];
	  float ty = y_positions[j];
	  if(!frames.count(tx) || !frames[tx].count(ty))
	    continue;
	  frames[tx][ty]->finalise(maxIntensity);    //
	}
      }
      for(uint i=0; i < x_positions.size(); i++){
	for(uint j=0; j < y_positions.size(); j++){
	  cout << "frame " << i << ", " << j << endl;
	  if(!frames.count(x_positions[i]) || !frames[ x_positions[i]].count(y_positions[j]))
	    continue;
	  vector<overlap_data*> o_data = frames[x_positions[i]][y_positions[j]]->adjustNeighbourPositions(15, rolloff, 40, 38, i, j);
	  for(uint k=0; k < o_data.size(); k++){
	    overlapData.push_back(o_data[k]);
	  }
	}
      }
      // at this point we want to make a new FileSetInfo, fill it up and then write to file.. 
      stackInfo = new FileSetInfo(waves, pixelWidth, pixelHeight);
      map<float, map<float, FrameStack*> >::iterator ot;
      map<float, FrameStack*>::iterator it;
      for(ot = frames.begin(); ot != frames.end(); ot++){
	for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
	  FrameStack* fstack = (*it).second;
	  FrameInfo* finfo = fstack->frameInfo();
	  stackInfo->addFrameInfo(finfo, (*ot).first, (*it).first);
	}
      }
      //      adjustStackBorders();
      stackInfo->writeInfo(infoFile.c_str());   // we should remember it as well, but hey who cares.. 
      
    }
    // then just set w, h, and d.. 
    w = frameWidth + x_positions.back() - x_positions[0];  // this is fine, doesn't have a problem with overlaps, since there isn't one at the end. .. 
    h = frameHeight + y_positions.back() - y_positions.front();
    d = z_positions.back() - z_positions.front();
    
     
    pw = (int) roundf( w / xscale );
    ph = (int) roundf( h / yscale );   // but it is not quite correct.. (but it may be good enough.. 
    cout << "Area covered : " << x << ", " << y << "  size : " << w << ", " << h << endl
	 << "Total number of stacks: " << x_positions.size() * y_positions.size() << endl
	 << "Each stack having dimensions : " << frameWidth << ", " << frameHeight << endl
	 << "containing                   : " << pixelHeight << ", " << pixelWidth << endl
	 << "For a total pixels of        : " << pw << " x " << ph << endl;
    cout << "And completeRectangle is : " << completeRectangle << endl;
    adjustStackBorders();

    // assign the map<fluorInfo, SLookup> luts structure to the frameStacks..
    // note that the frameIds structure gets set up as a consequence of adjustStackBorders() called above
    for(map<ulong, FrameStack*>::iterator it=frameIds.begin(); it != frameIds.end(); ++it)
      (*it).second->setLookupTables(&luts);

    return(true);  // lets see if we can handle incomplete rectangles
    return(completeRectangle);
}

void FileSet::adjustStackPosition(float xp, float yp, QPoint p){
  if(!(frames.count(xp) && frames[xp].count(yp))){
    cerr << "FileSet::adjustStackPosition no stack at : " << xp << "," << yp << endl;
  }
  // do something useful..
  frames[xp][yp]->adjustPosition(p);
  adjustStackBorders();
  //  setPosMap();
}

void FileSet::setPosMap(){
  cout << "setPosMap using positions " << 0 << "," << 0 << " : " << pw << "," << ph << endl;
  frameIds.clear();
  if(!framePosMap){
    framePosMap = new IdMap(0, 0, pw, ph);
  }else{
    framePosMap->reset(0, 0, pw, ph);
  }
  ulong id = 1;
  
  for(map<float, map<float, FrameStack*> >::iterator ot=frames.begin();
      ot != frames.end(); ++ot){
    for(map<float, FrameStack*>::iterator it=(*ot).second.begin();
	it != (*ot).second.end(); ++it){
      frameIds.insert(make_pair(id, (*it).second));
      it->second->setBorders();
      framePosMap->setId(id, it->second->left_border(), it->second->bottom_border(), 
			 it->second->right_border() - it->second->left_border(),
			 it->second->top_border() - it->second->bottom_border());
      id *= 2;
    }
  }

  // framePosMap will call setContribMap for each frameStack
  framePosMap->setContribMaps(frameIds, 15.0);

}

void FileSet::setPanelBias(unsigned int waveIndex, unsigned int column, unsigned int row, float scale, short bias){
  if(row < y_positions.size() && column < x_positions.size()){
    if(frames.count(x_positions[column]) && frames[x_positions[column]].count(y_positions[row]))
      frames[x_positions[column]][y_positions[row]]->setPanelBias(waveIndex, scale, bias);
  }
}

void FileSet::setBackgroundPars(unsigned int waveIndex, int xm, int ym, float qnt, bool bg_subtract){
  for(map<float, map<float, FrameStack*> >::iterator ot=frames.begin(); ot !=frames.end(); ++ot){
    for(map<float, FrameStack*>::iterator it = ot->second.begin(); it != ot->second.end(); ++it){
      it->second->setBackgroundPars(waveIndex, xm, ym, qnt, bg_subtract);
    }
  }
}

bool FileSet::setChannelOffsets(vector<ChannelOffset> offsets){
  bool ok = true;
  for(map<float, map<float, FrameStack*> >::iterator ot=frames.begin(); ot !=frames.end(); ++ot){
    for(map<float, FrameStack*>::iterator it = ot->second.begin(); it != ot->second.end(); ++it){
      if(!it->second->setChannelOffsets(offsets))
	ok = false;
    }
  }
  return(ok);
}

void FileSet::adjustStackBorders(){
  cout << "\nADJUST STACK BORDERS " << endl;
  for(map<float, map<float, FrameStack*> >::iterator ot=frames.begin();
      ot != frames.end(); ++ot){
    for(map<float, FrameStack*>::iterator it = (*ot).second.begin();
	it != (*ot).second.end(); ++it){
      cout << ".";
      (*it).second->setBorders();
    }
  }
  cout << endl;
  setPosMap();
}

bool FileSet::updateFileSetInfo(){
  if(stackInfo)
    return( stackInfo->writeInfo() );
  return(false);
}

// bool FileSet::readToRGB(float* dest, float xpos, float ypos, float dest_width, 
// 			float dest_height, unsigned int dest_pwidth, 
// 			unsigned int dest_pheight, unsigned int sliceNo,
// 			vector<channel_info> chinfo, raw_data* raw)
// //			float maxLevel, vector<float> bias, vector<float> scale, 
// //			vector<color_map> colors, bool bg_sub, raw_data* raw)
// {
//   // Note: I don't think this function is actually used anymore, as the we use pixel level positioning these days.
//   // so should try getting rid of it.. 
//     // the trickiest part is working out which framestack should contribute to each of these. However, maybe I don't have to do this
//     // here.
  
//   //**************************************
//   // Temporarily remove this to use 2 d background subtraction.
//   // If bg_sub is true, but we don't have any backgrounds, then set up the backgrounds
//   //  if(chinfo[0].bg_subtract && !backgrounds.size()){
//   // initBackgrounds();
//   //}

//     // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
//     int counter = 0;  // this just counts how many different framestacks contribute to the slice..
//     for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
// 	for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
// 	  if((*fit).second->readToRGB(dest, xpos, ypos, dest_width, dest_height, dest_pwidth, dest_pheight, sliceNo, 
// 				      chinfo, raw)){
// 		counter++;
// 	    }
// 	}
//     }
// //    cerr << "FileSet::readToRGB buffer read to from " << counter << " filestacks" << endl;
//     return(counter > 0);   // which is nicer than saying return counter.. because...
// }

bool FileSet::readToRGB(float* dest, unsigned int xpos, unsigned int ypos, 
			unsigned int dest_width, unsigned int dest_height, 
			unsigned int slice_no, vector<channel_info> chinfo,
			raw_data* raw){
  // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
  //****************** remove the below to use 2D background
  //  if(chinfo[0].bg_subtract && !backgrounds.size()){
  //  initBackgrounds();
  //}
  
  setLookupTables(chinfo);

  int counter = 0;  // this just counts how many different framestacks contribute to the slice..
  for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
    for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
      if((*fit).second->readToRGB(dest, xpos, ypos, dest_width, dest_height, 
				  slice_no, chinfo, raw)){
	counter++;
      }
    }
  }
  return(counter > 0);   // which is nicer than saying return counter.. because...
}

// bool FileSet::mip_projection(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int dest_pwidth, unsigned int dest_pheight,
// 			     float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, raw_data* raw)
// {
//   // the trickiest part is working out which framestack should contribute to each of these. However, maybe I don't have to do this
//   // here.
    
//     // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
//     int counter = 0;  // this just counts how many different framestacks contribute to the slice..
//     for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
// 	for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
// 	    if((*fit).second->mip_projection(dest, xpos, ypos, dest_width, dest_height, dest_pwidth, dest_pheight, maxLevel, bias, scale, colors, raw)){
// 		counter++;
// 	    }
// 	}
//     }
//     cerr << "FileSet::mip_projection buffer read to from " << counter << " filestacks" << endl;
//     return(counter > 0);   // which is nicer than saying return counter.. because...
// }

bool FileSet::mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
			     float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw)
{
  // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
  int counter = 0;  // this just counts how many different framestacks contribute to the slice..
  for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
    for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
      if((*fit).second->mip_projection(dest, xpos, ypos, dest_width, dest_height, maxLevel, bias, scale, colors, raw)){
	counter++;
      }
    }
  }
  return(counter > 0);   // which is nicer than saying return counter.. because...
}

ImStack* FileSet::imageStack(vector<unsigned int> wave_indices, bool use_cmap)
{
  return(imageStack(wave_indices, 0, 0, 0, pwidth(), pheight(), sectionNo(), use_cmap));
}

ImStack* FileSet::imageStack(stack_info sinfo, bool use_cmap)
{
  return(imageStack(sinfo.channels, sinfo.x, sinfo.y, sinfo.z, sinfo.w, sinfo.h, sinfo.d, use_cmap));
}

// the problem is that we don't have any channel_info (which includes colors and all sorts of nice things.
ImStack* FileSet::imageStack(vector<unsigned int> wave_indices, int x, int y, int z,
			     unsigned int w, unsigned int h, unsigned int d, bool use_cmap)
{
  if(!w || !h || !d)
    return(0);
  vector<unsigned int> wi;  // the ones we use
  vector<channel_info> ch_info;
  for(uint i=0; i < wave_indices.size(); ++i){
    if(wave_indices[i] < flInfo.size()){
      wi.push_back(wave_indices[i]);
      ch_info.push_back( channel_info(wave_indices[i], channelInfo(wave_indices[i])) );
    }
  }
  if(!wi.size())
    return(0);
  float** stack_data = new float*[wi.size()];
  for(uint i=0; i < wi.size(); ++i){
    stack_data[i] = new float[ w * h * d ];
    memset((void*)stack_data[i], 0, sizeof(float) * w * h * d);
    readToFloat(stack_data[i], x, y, z, w, h, d, wi[i], use_cmap);
  }
  ImStack* imStack = new ImStack(stack_data, ch_info, x, y, z, w, h, d);
  return(imStack);
}

bool FileSet::readToFloat(float* dest, int xb, int yb, int zb, int pw, int ph, int pd, unsigned int waveIndex, bool use_cmap){
    // first do some sanity checks..
    if(!(pw > 0 && ph > 0 && pd > 0)){
	cerr << "FileSet::readToFloat at least one dimension is non positive : " << pw << ", " << ph << ", " << pd << endl;
	return(false);
    }
    // Check that we have the appropriate sections available..
    // zb must not be negative and zb + pd <= number of frames.. 
    if(zb < 0){ // The function we call in the frameStack actually uses an unsigned value.. but use of unsigned values is actually not so good so.. 
	cerr << "FileSet::readToFloat negative z values currently unsupported .. so sorry .. " << endl;
	return(false);
    }
    
    bool gotSomething = false;
    map<float, map<float, FrameStack*> >::iterator ot;
    map<float, FrameStack*>::iterator it;
    for(ot = frames.begin(); ot != frames.end(); ot++){
	for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
	  if((*it).second->readToFloat(dest, xb, pw, yb, ph, zb, pd, waveIndex, maxIntensity, use_cmap)){
		gotSomething = true;
	    }
	}
    }
    return(gotSomething);
}	

bool FileSet::readToShort(unsigned short* dest, int xb, int yb, unsigned int slice, int pw, int ph, unsigned int waveIndex){
  bool gotSomething = false;
  map<float, map<float, FrameStack*> >::iterator ot;
  map<float, FrameStack*>::iterator it;
  for(ot = frames.begin(); ot != frames.end(); ot++){
    for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
      if((*it).second->readToShort(dest, xb, pw, yb, ph, slice, waveIndex)){
	gotSomething = true;
      }
    }
  }
  return(gotSomething);
}

bool FileSet::readToShort(unsigned short* dest, unsigned int c, unsigned int r, unsigned int slice, unsigned int waveIndex){
  if(c >= x_positions.size() || r >= y_positions.size()){
    cerr << "FileSet::readToShort c or r too large: " << c << ", " << r << endl;
    return(false);
  }
  if(frames.count(x_positions[c]) && frames[x_positions[c]].count(y_positions[r]))
    return(frames[x_positions[c]][y_positions[r]]->readToShort(dest, slice, waveIndex));

  cerr << "Unable to locate frame at pos: " << c << "," << r << "  : " << x_positions[c] << "," << y_positions[r] << endl;
  return(false);
}

stack_stats FileSet::stackStats(unsigned int c, unsigned int r, int xb, int yb,
					int s_width, int s_height, unsigned int waveIndex)
{
  stack_stats empty_stats;
  if(c >= x_positions.size() || r >= y_positions.size()){
    cerr << "FileSet::stackStats c or r too large: " << c << ", " << r << endl;
    return(empty_stats);
  }
  if(frames.count(x_positions[c]) && frames[x_positions[c]].count(y_positions[r]))
    return(frames[x_positions[c]][y_positions[r]]->stackStats(xb, yb, s_width, s_height, waveIndex));
  return(empty_stats);
}


stack_stats FileSet::stackStats(unsigned int c, unsigned int r, int xb, int yb, int zb,
				int s_width, int s_height, int s_depth, unsigned int waveIndex)
{
  stack_stats empty_stats;
  if(c >= x_positions.size() || r >= y_positions.size()){
    cerr << "FileSet::stackStats c or r too large: " << c << ", " << r << endl;
    return(empty_stats);
  }
  if(frames.count(x_positions[c]) && frames[x_positions[c]].count(y_positions[r]))
    return(frames[x_positions[c]][y_positions[r]]->stackStats(xb, yb, zb, s_width, s_height, s_depth, waveIndex));
  return(empty_stats);
}


void FileSet::borders(int& left, int& right, int& bottom, int& top){
    bool gotSomething = false;
    map<float, map<float, FrameStack*> >::iterator ot;
    map<float, FrameStack*>::iterator it;
    int l, r, b, t;
    for(ot = frames.begin(); ot != frames.end(); ot++){
	for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
		l = (*it).second->left_border();
		r = (*it).second->right_border();
		b = (*it).second->bottom_border();
		t = (*it).second->top_border();
		if(!gotSomething){
		    left = l;
		    right = r;
		    bottom = b;
		    top = t;
		    gotSomething = true;
		}
		if(l < left)
		    left = l;
		if(r > right)
		    right = r;
		if(b < bottom)
		    bottom = b;
		if(t > top)
		    top = t;
	}
    }
}


bool FileSet::readToFloatPro(float* dest, int xb, int iwidth, int yb, int iheight, int wave){
    // check that width and height are positive..
    if(iwidth <= 0 || iheight <= 0){
	cerr << "FileSet::readToFloatPro width or height is negative " << iwidth << ", " << iheight << endl;
	return(false);
    }
    // It should be noted that strictly speaking we should be able to use negative coordinates,
    // but that the current implementation of FrameStack isn't smart enough to handle this 
    // (hence all those unsigned ints in the prototype). This is bad, and a result of my 
    // temporary enthusiasm for unsigned integers.. 
    if(xb < 0){
	cerr << "FileSet::readTofloatPro adjusted xb from " << xb << "  to 0" << endl;
	xb = 0;
    }
    if(yb < 0){
	cerr << "FileSet::readToFloatPro adjusted yb from " << yb << "  to 0" << endl;
	yb = 0;
    }
    bool gotSomething = false;
    map<float, map<float, FrameStack*> >::iterator ot;
    map<float, FrameStack*>::iterator it;
    // note that readToFloatPro deals only with local coordinates.. -- (why ?)
    // This is a major bummer, but I'll have to correct for that here.. 

    for(ot = frames.begin(); ot != frames.end(); ot++){
	for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
	    if((*it).second->readToFloatProGlobal(dest, xb, iwidth, yb, iheight, wave)){
		gotSomething = true;
	    }
	}
    }
    return(gotSomething);
}

BorderInfo* FileSet::borderInformation(float x, float y){
  if(frames.count(x) && frames[x].count(y))
    return(frames[x][y]->borderInformation());
  cerr << "FileSet::borderInformation uknown frame location at : " << x << "," << y << endl;
  return(0);
}

float* FileSet::paintCoverage(int& w, int& h, float maxCount){
  return( framePosMap->paintCountsToRGB(w, h, maxCount) );
}

void FileSet::determineZOffsets(){
    // for now do nothing..
    if(!completeRectangle){
	cerr << "FileSet::determineZOffsets called, but rectangle is not complete. bummer, " << endl;
    }

    // this is ugly, but the best I can think of for the moment..

    for(uint i=0; i < y_positions.size(); i++){
	vector<weightedValueSet> contrasts;
	float ty = y_positions[i];
	contrasts.resize(waves.size());
	for(uint j=0; j < x_positions.size(); j++){
	    float tx = x_positions[j];
	    cout << "Determining Focal Plane for Framestack at : " << j << ", " << i << endl;
	    frames[tx][ty]->determineFocalPlanes(32);
	    vector<vector<float> > fc = frames[tx][ty]->contrastData();
	    if(fc.size() == contrasts.size()){
		for(uint k=0; k < fc.size(); ++k){
		    for(uint l=0; l < fc[k].size(); ++l){
			contrasts[k].x.push_back((float)j);
			contrasts[k].y.push_back((float)l);
			contrasts[k].w.push_back(fc[k][l]);
		    }
		}
	    }else{
		cerr << "FileSet::determineZOffsets contrasts is not the same size as fs (" << contrasts.size() << " != " << fc.size() << endl;
	    }
	}
	// and at this position we can get the appropriate equation from fit_weigted_linear..  (maybe.. ?)
	for(uint k=0; k < contrasts.size(); k++){
	    lin_function lf = fit_weighted_linear(contrasts[k].y, contrasts[k].x, contrasts[k].w);
	    cout << "Row : " << i << "  Wavelength : " << waves[k] << "  Equation is : y = " << lf.a << " + " << lf.b << "x" << endl;
	    for(uint j=0; j < x_positions.size(); ++j){
		float tx = x_positions[j];
		frames[tx][ty]->setFocalPlane(k, lf.a + lf.b * float(j));
	    }
	}
    }
}

void FileSet::initBackgrounds(){
  // do only if we don't have any backgrounds.
  cout << "Calling init backgrounds" << endl;
  if(backgrounds.size())
    return;
  int wcounter = 0;           // horrible kludge warning 
  for(set<fluorInfo>::iterator it=flInfo.begin(); it != flInfo.end(); it++){
    ImageData* id = new ImageData(this);
    Background* bg = new Background(id, 16, 16, 8, 75);  // default background parameters
    backgrounds.insert(make_pair((*it), bg));
    cout << "setting background for " << wcounter << endl;
    bg->setBackground(wcounter);
    wcounter++;
  }
  // The backgrounds will need to be passed on to the frame-stacks, and so on..
  // frameStacks are arranged by x and y position. hence a painful double map
  for(map<float, map<float, FrameStack*> >::iterator it=frames.begin();
      it != frames.end(); it++){
    for(map<float, FrameStack*>::iterator jt=it->second.begin();
	jt != it->second.end(); jt++){
      jt->second->setBackgrounds(backgrounds);
    }
  }
}

void FileSet::initBackgrounds(map<fluorInfo, backgroundPars> bgp){
  if(backgrounds.size()){
    setBackgroundParameters(bgp);
    return;
  }  
  int wcounter = 0;           // horrible kludge warning 
  for(set<fluorInfo>::iterator it=flInfo.begin(); it != flInfo.end(); it++){
    ImageData* id = new ImageData(this);
    Background* bg = 0;
    if(!bgp.count((*it))){
      bg = new Background(id, 16, 16, 8, 75);  // default background parameters
    }else{
      bg = new Background(id, bgp[(*it)]);
    }
    backgrounds.insert(make_pair((*it), bg));
    cout << "setting background for " << wcounter << endl;
    bg->setBackground(wcounter);
    wcounter++;
  }
  // The backgrounds will need to be passed on to the frame-stacks, and so on..
  // frameStacks are arranged by x and y position. hence a painful double map
  for(map<float, map<float, FrameStack*> >::iterator it=frames.begin();
      it != frames.end(); it++){
    for(map<float, FrameStack*>::iterator jt=it->second.begin();
	jt != it->second.end(); jt++){
      jt->second->setBackgrounds(backgrounds);
    }
  }
  
}

// Problematic function since we hold the lookup table objects as
// a map<fluorInfo, SLookup*> collection.
// The following is how it has been done in the FileSet object
void FileSet::setLookupTables(std::vector<channel_info>& ch_info)
{
  unsigned int i=0;
  for(map<fluorInfo, SLookup*>::iterator it=luts.begin(); it != luts.end(); ++it){
    if(i >= ch_info.size()){
      cerr << "FileSet::setLookupTables Not enough channel_info objects in ch_info: i " << i << " size: " << ch_info.size() << endl;
      break;
    }
    (*it).second->setPars(ch_info[i]);
    ++i;
  }
  if(i < luts.size())
    cerr << "FileSet::setLookupTables not enough luts : ch_info.size() " << ch_info.size() << " > " << i << endl;
}

void FileSet::setBackgroundParameters(map<fluorInfo, backgroundPars> bgp){
  if(!backgrounds.size()){
    initBackgrounds(bgp);
    return;
  }
  for(map<fluorInfo, Background*>::iterator it=backgrounds.begin();
      it != backgrounds.end(); ++it){
    if(!bgp.count(it->first)){
      cerr << "FileSet::setBackgroundParameters : missing parameter set" << endl;
      continue;
    }
    it->second->setParameters(bgp[it->first]);
    // that will actually do everything we need to do.
  }  
}

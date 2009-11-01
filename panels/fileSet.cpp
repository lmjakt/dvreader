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

#include "fileSet.h"
#include "fileSetInfo.h"
#include "../stat/stat.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <math.h>

using namespace std;

FileSet::FileSet(int* waveLengths, int waveNo, float maxLevel){
    wave_lengths = waveLengths;
    wave_no = waveNo;
    maxIntensity = maxLevel;
    waves.resize(waveNo);
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
}

bool FileSet::addFrame(string fname, ifstream* in, size_t framePos, size_t readPos, size_t extHeadSize,
		       short numInt, short numFloat, unsigned short byteSize,
		       bool real, bool bigEnd, unsigned int width, unsigned int height, float dx, float dy, float dz)
{
    // make a new frame, and if its x and y coordinates don't fit then 
    // make a new framestack and stick it into that ..

    if(!fileName){
	fileName = fname.c_str();
    }

    // first use a temporary ifstream as it seems we can't make and delete thousands of ifstreams..
    Frame* frame = new Frame(in, framePos, readPos, extHeadSize, numInt, numFloat, byteSize, real, bigEnd, width, height, dx, dy, dz);
    if(!frame->ok()){
	cerr << "FileSet::addFrame Frame is not ok" << endl;
	delete frame;
	return(false);
    }
    flInfo.insert(fluorInfo(frame->excitation(), frame->emission(), frame->exposure()));

    // and then look for a suitable framestack..
    bool stackIsNew = false;
    float x_pos = frame->xPos();
    float y_pos = frame->yPos();   // these get modified by the getStack function to point to the appropriate things for assigning the stack

    FrameStack* fstack;

    if(!getStack(x_pos, y_pos)){
	stackIsNew = true;
	cout << "no stack found for frame counter  "  << ifstreamCounter << endl;
	ifstream* newStream = new ifstream(fname.c_str());
	ifstreamCounter++;
	if(!(*newStream)){
	    cerr << "FileSet::addFrame failed to create new filestream on new stack counter " << ifstreamCounter << endl;
	    exit(1);
	}
	fstack = new FrameStack(wave_lengths, wave_no, newStream, maxIntensity);   // in which case we should not delete or change the ifstream around.. 
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
	cout << "adding first framestack to FileSet position : " << x << ", " << y << ", " << frame->zPos() << "  dims : " << pixelWidth << ", " << pixelHeight << endl;

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
    z_positions.assign(z_set.begin(), z_set.end());   // I have a feeling that these vectors may not be very useful, but .. you never know..
    
    // these values are likely to be useful. 
    float xscale = frameWidth / float(pixelWidth);
    float yscale = frameHeight / float(pixelHeight);

    // to be complete, we essentially have to have a thingy at each x and y position.. and there should always be an overlap..
    // that is the simplest thing to take into account..
    completeRectangle = true;

    frameNo = frames[x_positions[0]][y_positions[0]]->frameNo();

    for(uint i=0; i < x_positions.size(); i++){
	if(!frames.count(x_positions[i])){
	    cerr << "No framestacks defined for x position x: " << x_positions[i] << endl;  // though this shouldn't be possible..
	    completeRectangle = false;
	    continue;
	}
	map<float, map<float, FrameStack*> >::iterator it = frames.find(x_positions[i]);
	for(uint j=0; j < y_positions.size(); j++){
//	    cout << "Frame Stack at : " << x_positions[i] << ", " << y_positions[j] << endl;
	    if(!(*it).second.count(y_positions[j])){
		cerr << "No framestacks defined for y_positions y: " << y_positions[j] << endl;
		completeRectangle = false;
	    }else{
		//	frames[x_positions[i]][y_positions[j]]->finalise(maxIntensity);  // here I should also pass the projection data if it exists.. 
	
		// also set the pixelPosition of the thingy..
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
    
    if(completeRectangle){
	for(uint i=0; i < x_positions.size(); i++){
	    for(uint j=0; j < y_positions.size(); j++){
		float tx = x_positions[i];
		float ty = y_positions[j];
//		frames[tx][ty]->finalise(maxIntensity);    //
//		cout << "Determining Focal Plane for Framestack at : " << i << ", " << j << endl;
//		frames[tx][ty]->determineFocalPlanes(32);
		//
		//	cout << "setting neighbours for panel at " << i << ", " << j << endl;
		if(i){
		    //   cout "i : " << i << endl;
		    float lx = x_positions[i-1];  // left x..
			if(!frames[tx][ty]->setNeighbour(frames[lx][ty], 0)){
			    cerr << "no overlap between " << i << "," << j << "  and  " << i-1 << "," << j << endl;
			}
		}
		if(j){
		    //cout << "j : " << j << endl;
		    float by = y_positions[j-1];  // below y.. 
		    if(!frames[tx][ty]->setNeighbour(frames[tx][by], 3)){   // the function is recipocral, so this is all we need to do..
			cerr << "no overlap between " << i << "," << j << "  and  " << i << "," << j-1 << endl;
		    }
		}
	    }
	}
    }
    // first check if there is afile..
    string infoFile = fileName;
    infoFile.append(".info");
    
    FileSetInfo stackInfo(infoFile.c_str());
    if(stackInfo.ok){
	cout << "Managed to read fileSetInfo from  " << infoFile << endl;
	// Read pixel positions of files and other things and set up the 
	// projections by calling finalise() with the appropriate float** structure..
	map<float, map<float, FrameStack*> >::iterator ot;
	map<float, FrameStack*>::iterator it;
	for(ot = frames.begin(); ot != frames.end(); ot++){
	    for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
		FrameStack* fstack = (*it).second;
		FrameInfo* finfo = stackInfo.getStack((*ot).first, (*it).first);
		if(finfo){
		    cout << "Setting stack positions to : " << finfo->xp << ", " << finfo->yp << endl;
//		    fstack->setPixelPos(finfo->xp, finfo->yp, false);
		    
		    fstack->finalise(maxIntensity, finfo);
//		    fstack->finalise(maxIntensity, finfo->projection);
		}
	    }
	}
    }else{
	// We should do this even if the rectangle is not complete, but we have to be a bit more careful about how we set things
	// up.. -- need to put in some more things in there.. 

	// set up the overlaps. Only do this if the rectangle is complete, -otherwise it is a little bit too difficult..
	// make sure to make FileSetInfo and get that to write out the info so we don't have to do this again.. 
	for(uint i=0; i < x_positions.size(); i++){
	    for(uint j=0; j < y_positions.size(); j++){
		float tx = x_positions[i];
		float ty = y_positions[j];
		frames[tx][ty]->finalise(maxIntensity);    //
		//cout << "\n\tADJUSTING POSITIONS" << endl;
	    }
	}
	for(uint i=0; i < x_positions.size(); i++){
	    for(uint j=0; j < y_positions.size(); j++){
		cout << "frame " << i << ", " << j << endl;
		vector<overlap_data*> o_data = frames[x_positions[i]][y_positions[j]]->adjustNeighbourPositions(15, 18, 32, 30, i, j);
		for(uint k=0; k < o_data.size(); k++){
		    overlapData.push_back(o_data[k]);
		}
	    }
	}
	// at this point we want to make a new FileSetInfo, fill it up and then write to file.. 
	stackInfo = FileSetInfo(waves, pixelWidth, pixelHeight);
	map<float, map<float, FrameStack*> >::iterator ot;
	map<float, FrameStack*>::iterator it;
	for(ot = frames.begin(); ot != frames.end(); ot++){
	    for(it = (*ot).second.begin(); it != (*ot).second.end(); it++){
		FrameStack* fstack = (*it).second;
		cout << "Making a new FrameInfo at position : " << fstack->left() << ", " << fstack->bottom() << "  ( " << fstack->x_pos() << ", " << fstack->y_pos() << " ) " << endl;
		
		FrameInfo* finfo = fstack->frameInfo();
//		FrameInfo* finfo = new FrameInfo(stackInfo.waveNo, fstack->projectionData(), fstack->x_pos(), fstack->y_pos(),  fstack->left(), fstack->bottom(), fstack->p_width(), fstack->p_height());
		stackInfo.addFrameInfo(finfo, (*ot).first, (*it).first);
	    }
	}
	stackInfo.writeInfo(infoFile.c_str());   // we should remember it as well, but hey who cares.. 
	
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
    
    return(completeRectangle);
}

bool FileSet::readToRGB(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int dest_pwidth, unsigned int dest_pheight, unsigned int sliceNo,
			float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, raw_data* raw)
{
    // the trickiest part is working out which framestack should contribute to each of these. However, maybe I don't have to do this
    // here.
    
    // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
    int counter = 0;  // this just counts how many different framestacks contribute to the slice..
    for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
	for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
	    if((*fit).second->readToRGB(dest, xpos, ypos, dest_width, dest_height, dest_pwidth, dest_pheight, sliceNo, maxLevel, bias, scale, colors, raw)){
		counter++;
	    }
	}
    }
//    cerr << "FileSet::readToRGB buffer read to from " << counter << " filestacks" << endl;
    return(counter > 0);   // which is nicer than saying return counter.. because...
}

bool FileSet::readToRGB(float* dest, unsigned int xpos, unsigned int ypos, unsigned int dest_width, unsigned int dest_height, unsigned int slice_no, 
			float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, raw_data* raw){
    // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
    int counter = 0;  // this just counts how many different framestacks contribute to the slice..
    for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
	for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
	    if((*fit).second->readToRGB(dest, xpos, ypos, dest_width, dest_height, slice_no, maxLevel, bias, scale, colors, raw)){
		counter++;
	    }
	}
    }
//    cerr << "FileSet::readToRGB buffer read to from " << counter << " filestacks" << endl;
    return(counter > 0);   // which is nicer than saying return counter.. because...
}

bool FileSet::mip_projection(float* dest, float xpos, float ypos, float dest_width, float dest_height, unsigned int dest_pwidth, unsigned int dest_pheight,
			     float maxLevel, vector<float> bias, vector<float> scale, vector<color_map> colors, raw_data* raw)
{
    // the trickiest part is working out which framestack should contribute to each of these. However, maybe I don't have to do this
    // here.
    
    // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
    int counter = 0;  // this just counts how many different framestacks contribute to the slice..
    for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
	for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
	    if((*fit).second->mip_projection(dest, xpos, ypos, dest_width, dest_height, dest_pwidth, dest_pheight, maxLevel, bias, scale, colors, raw)){
		counter++;
	    }
	}
    }
    cerr << "FileSet::mip_projection buffer read to from " << counter << " filestacks" << endl;
    return(counter > 0);   // which is nicer than saying return counter.. because...
}

bool FileSet::mip_projection(float* dest, int xpos, int ypos, unsigned int dest_width, unsigned int dest_height,
			     float maxLevel, std::vector<float> bias, std::vector<float> scale, std::vector<color_map> colors, raw_data* raw)
{
    // the trickiest part is working out which framestack should contribute to each of these. However, maybe I don't have to do this
    // here.
    
    // given that each framestack knows it's bordering frame stacks I can just ask them to work it out themselves..
    int counter = 0;  // this just counts how many different framestacks contribute to the slice..
    for(map<float, map<float, FrameStack*> >::iterator it=frames.begin(); it != frames.end(); it++){
	for(map<float, FrameStack*>::iterator fit=(*it).second.begin(); fit != (*it).second.end(); fit++){
	    if((*fit).second->mip_projection(dest, xpos, ypos, dest_width, dest_height, maxLevel, bias, scale, colors, raw)){
		counter++;
	    }
	}
    }
    cerr << "FileSet::mip_projection buffer read to from " << counter << " filestacks" << endl;
    return(counter > 0);   // which is nicer than saying return counter.. because...
}

bool FileSet::readToFloat(float* dest, int xb, int yb, int zb, int pw, int ph, int pd, unsigned int waveIndex){
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
	    if((*it).second->readToFloat(dest, xb, pw, yb, ph, zb, pd, waveIndex, maxIntensity)){
		gotSomething = true;
	    }
	}
    }
    return(gotSomething);
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

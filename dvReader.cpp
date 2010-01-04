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

#include "dvReader.h"
#include <string>
#include <qstring.h>
#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <math.h>
#include <time.h>
#include <stdio.h>

using namespace std;

const int maxSignal = 4096; // for the camera we are using
                            // if the measurement is a short. 
                            // constrain floats between 0 and 1.. 

DVReader::DVReader(const char* fName, float maxlevel){
    // set some values just in case..
    margin = 50;
    //imageData = 0;
    currentSection = 0;
    // try to guess what the format is..
    // just from the file name..
    FileFormat fm = DELTAVISION; // the default.. 
    string fn = fName;
    if(fn.find(".ics") != string::npos){
	fm = ICS_V1;
    }
    maxLevel = maxlevel;
//  cout << "fileFormat is : " << fm << endl;
    sameEndian = true;   // but change if later.. 
    readDataFromFile(fName, fm);
    currentSection = -1;
}


bool DVReader::readDataFromFile(const char* fName, FileFormat format){
    if(format == DELTAVISION){
	return(readDVFile(fName));
    }
    if(format == ICS_V1){
	return(readICSv1File(fName));
    }
    cerr << "Unknown file format : " << format << endl;
    return(false);
}

bool DVReader::readDVFile(const char* fName){
    //
    headerSize = 1024;   // use for seeking to appropriate place... 


///  The below comment is untrue. I cannot open up an unlimited number of file handles. So I think I ended up
///  using one for each thingy.. 
//// It seems that there is no problem to open up to 50,000 handles on a given file. That is useful, since it should
//// mean that we can make an object for every frame. This frame has all the information that we need to extract data..
//// 
    

    ifstream in(fName, ios::binary);
    if(!in){
	cerr << "couldn't open file: " << fName << endl;
	return(false);
    }
    // and then read in the header.. lots of trouble..
    in.read((char*)&nx, 4);
    in.read((char*)&ny, 4);

    // if nx and ny are nonsensical, try and change the endianNess.
    // arbitrary size limit between 1 -> 4000 for both, not perfect but we should catch most )
    int maxSize = 4048;
    sameEndian = true;
    if(nx <= 0 || nx > maxSize || ny <= 0 || ny > maxSize){
	cerr << "dimensions of image seem strange : " << nx << ", " << ny << "\t(current max dimension : " << maxSize << endl;
	cerr << "will try to swap bytes : " << endl;
	int snx, sny;
	swapBytes((char*)&nx, (char*)&snx, 1, 4);
	swapBytes((char*)&ny, (char*)&sny, 1, 4);
	if(snx <= 0 || snx > maxSize || sny <= 0 || sny > maxSize){
	    cerr << "dimensions still no good after swapping bytes : " << snx << ", " << sny << endl;
	    exit(1);
	}
	nx = snx;
	ny = sny;
	sameEndian = false;
    }

    readInt(in, nsec);
//    cout << "and nsec is : " << nsec << " sameEndian is : " << sameEndian << endl;
    readInt(in, mode);
    readInt(in, mxst);
    readInt(in, myst);
    readInt(in, mzst);
    readInt(in, mx);
    readInt(in, my);
    readInt(in, mz);
    

    
    cout << "nx : " << nx << endl
	 << "ny : " << ny << endl
	 << "nsec : " << nsec << endl
	 << "mxst : " << mxst << endl
	 << "myst : " << myst << endl;
    
    
    readFloat(in, dx);
    readFloat(in, dy);
    readFloat(in, dz);
    readFloat(in, alpha);
    readFloat(in, beta);
    readFloat(in, gamma);


    
    readInt(in, xaxis);
    readInt(in, yaxis);
    readInt(in, zaxis);


    
    readFloat(in, min);
    readFloat(in, max);
    readFloat(in, mean);


    
    // type and nspg are a little bit strange as they appear to be different to the specfication 
    // which specifies nspg, next, dvid after mean, with type appearing much later on 
    // since this seems to work, I'll leave it at as is.. 

    readShort(in, type);
    readShort(in, nspg);
    

    readInt(in, next);
    readShort(in, dvid);

    // leave the below as a blank.. 
    in.read((char*)&blank, 30);

    readShort(in, nint);     // this number might tell us whether or not we are dealing with raw data.. 
    readShort(in, nreal);
    readShort(in, sub);
    readShort(in, zfac);

    readFloat(in, min2);
    readFloat(in, max2);
    readFloat(in, min3);
    readFloat(in, max3);
    readFloat(in, min4);
    readFloat(in, max4);


    readShort(in, type_2);
    readShort(in, lensid);
    readShort(in, n1);
    readShort(in, n2);
    readShort(in, v1);
    readShort(in, v2);
    
    readFloat(in, min5);
    readFloat(in, max5);


    readShort(in, nt);
    readShort(in, interleaving);
    
    
    readFloat(in, xTilt);
    readFloat(in, yTilt);
    readFloat(in, zTilt);
    
    
    readShort(in, nw);
    readShort(in, wave1);
    readShort(in, wave2);
    readShort(in, wave3);
    readShort(in, wave4);
    readShort(in, wave5);
    

    // and for convenience later on..
    waves[0] = (int)wave1;
    waves[1] = (int)wave2;
    waves[2] = (int)wave3;
    waves[3] = (int)wave4;
    waves[4] = (int)wave5;  // ok.. 
    // and lets do the same for the mins and maxes/...
    mins[0] = min;
    mins[1] = min2;
    mins[2] = min3;
    mins[3] = min4;
    mins[4] = min5;
    maxs[0] = max;
    maxs[1] = max2;
    maxs[2] = max3;
    maxs[3] = max4;
    maxs[4] = max5;
    // and let's initialise the colors. make em all 0 to begin with..
    for(int i=0; i < nw; i++){    // only both with ones that we have..
	for(int j=0; j < 3; j++){
	    waveColors[i][j] = 0.0;   // start with black..
	}
    }
    
    readFloat(in, x0);
    readFloat(in, y0);
    readFloat(in, z0);
    
    
    readInt(in, nttl);  // the number of titles.. 

    // at whichy point we should go further.. 
    // the data should start at next + headerSize 
    // and basically will take up 
    //
    // nx * ny * 2
    //
    // bytes for each section. We don't actually know how the sections are arranged yet, but this is something
    // which I'm aiming to find out, by looking at them.
    // at this point.. let's simply define the imageData..

    // if we are dealing with raw data then we will have one extra value for each 
    bool isReal = false;
    switch(mode){
	case 0:
	    pSize = 1;
	    isReal = false;
	    break;
	case 1:
	    pSize = 2;
	    isReal = false;
	    break;
	case 2:
	    pSize = 4;
	    isReal = true;
	    break;
	case 3:
	    pSize = 4;
	    isReal = false;   // complex numbers are not very real.. 
	    break;
	case 4:
	    pSize = 8;
	    isReal = true;   // but troublesome indeed.. 
	    break;
	default :
	    pSize = 2;
	    cerr << "Unknown mode : " << mode << "\tassuming 2 bytes per pixel" << endl;
    }
    
    sectionSize = nx * ny * pSize;
    // seek to the appropriate position..
    in.seekg(headerSize + (std::ios::pos_type)next);
    // this is a bit ugly, but I believe that this is the only way in which we can do this...
    secNo = nsec/nw;   // the real number of sections..

    //float* floatData = new float[nw * nx * ny];   // an array for each frame containing data 


    
    std::ios::pos_type secSize = (pSize * nx * ny);  // the number of bytes per image..
    //<<<<<<< HEAD:dvReader.cpp
    //std::ios::pos_type imBegin = headerSize + next;  // next = n extended.. 
    //=======
    std::ios::pos_type imBegin = headerSize + (std::ios::pos_type)next;  // next = n extended.. 
    //>>>>>>> 3D_bg_subtract:dvReader.cpp
    
    // let's have a look first ..
    cout << "next : " << next << "\tnint : " << nint << "\tnreal : " << nreal << endl;
    cout << "interleaving : " << interleaving << endl;
    // exit(1);

    // First make the fileSet structure, then read the header and fill in all the bits of the file set..
    // 
    fileSet = new FileSet(waves, (int)nw, maxLevel);
    // and let's have a temporary filestream..
    ifstream* in2 = new ifstream(fName);
    if(!(*in2)){
	cerr << "dvReader failed to create a second filestream on " << fName << endl;
	exit(1);
    }
    // ok and now let's read the extendned header ..
    int tempInt;
    float tempFloat;   // assume 
    in.seekg(headerSize);
    for(int i=0; i < nsec; i++){
	std::ios::pos_type framePos = imBegin + secSize * i;
	//<<<<<<< HEAD:dvReader.cpp
	//std::ios::pos_type readPos = headerSize + (nint + nreal) * 4 * i;
	//=======
	std::ios::pos_type readPos = headerSize + (std::ios::pos_type)((nint + nreal) * 4 * i);
	//>>>>>>> 3D_bg_subtract:dvReader.cpp
	std::ios::pos_type extHeadSize = next;  // I think.. 
	bool bigEnd = !sameEndian;   // but this is indeed very bad way of doing it.. 
	if(!fileSet->addFrame(fName, in2, framePos, readPos, extHeadSize, nint, nreal, pSize, isReal, bigEnd, nx, ny, dx, dy, dz)){
	    cerr << "DVreader unable to add frame to file set will die here for debugging.." << endl;
	    exit(1);
	}
//	cout << i << "\ti:";
	for(int j=0; j < nint; j++){
	    in.read((char*)&tempInt, 4);
//	    cout << "\t" << tempInt;
	}
//	cout << endl << "\tf:";
	for(int j=0; j < nreal; j++){
	    in.read((char*)&tempFloat, 4);
//	    cout << "\t" << tempFloat;
	}
//	cout << endl << endl;
    }
    delete in2;

    // and here let's run finalise..
    if(!fileSet->finalise()){
	cerr << "File set did not finalise satisfactorily" << endl;
	exit(1);
    }


    vector<float> biasFactors(nw);
    vector<float> scaleFactors(nw);

    //   float* image_data = new float[nw * nx * ny * secNo];
    for(int i=0; i < nw; i++){
	biasFactors[i] = 0;
	scaleFactors[i] = 1;
  
    }

    return(true);
}

void DVReader::swapBytes(char* data, unsigned int wn, unsigned int ws){    // swaps in place
    char* word = new char[ws];  // the word size.. 
    for(uint i=0; i < wn; i++){
	for(uint j=0; j < ws; j++){
	    word[j] = data[(i+1) * ws - (j+1)];
	}
	for(uint j=0; j < ws; j++){
	    data[i * ws + j] = word[j];
	}
    }
    delete word;
}

vector<string> DVReader::getWords(char ws, char ls, ifstream& in){
    // First get a line, then make an istringstream from this,, and use that..
    // to read the words..
    vector<string> words;
    string word;
    string line;
    getline(in, line, ls);   // ls line separator..
    //cout << "line : " << line << endl;
    istringstream is(line);
    while(getline(is, word, ws)){
	//cout << "word : " << word << endl; 
	words.push_back(word);
    }
    return(words);
}

bool DVReader::readICSv1File(const char* fName){
    // ics version 1 consists of two files, one header file (the .ics) and one file
    // containing the actual data.. (the .ids) file. The name of the .ids file should
    // be identical to the ics file, so we need to do a bit of string handling to cope
    cout << "read ics version 1 file format file : " << fName << endl;
    string idsFile = fName;   // according the format this should have the extension .ids
    /// some parameters that we need to set from the file...
    // nx, ny, secNo, dx, dy, dz -- see the line layout sizes and parameter scale, and paramter units.. 
    nx = ny = secNo = 1;
    int nt = 1;   // time positions number of.. 
    dx = dy = dz = 1.0;
    nw = 0;   // the wave no..
    int paramNo = 0;
    vector<string> order;
    int bitNo = 0;
    int sigBits = 0;
    int repFormat = -1;   // char = 0 short = 1, int = 2, float = 3;   // and if not the case.. then .. 
    bool isSigned;        // is the data signed or not.. 
    bool compressed = false;
    bool bigEnded = true;    // is it big ended. As I'm not likely to compile this for other machines.
    // defaults to true as we can check for small endianness very easily.. 
    // I will assume that the host is small ended... 
    // there's other stuff as well, but I think that this will do for now..

    int t_dim[5];   // a dimension translation
    int dims[5];    // the sizes of the dimensions in the order in the file...
    for(int i=0; i < 4; i++){
	t_dim[i] = dims[i] = 1;
    }

    // note our internal order is always : x, y, z, w.  so we need to translate things..

    ifstream in(fName);
    if(!fName){
	return(false);
    }
    char ws, ls;  // delimiter and el.. used for splitting the files..
    in.read(&ws, 1);
    in.read(&ls, 1);
    cout << "word seperator :" << ws << ": line separator:" << ls << endl;
    // these will be used in the parsing of the rest of the file..
    while(in.good()){
	vector<string> words = getWords(ws, ls, in);
	// just ooutpot to make sure its working..
	for(uint i=0; i < words.size(); i++){
	    cout << words[i] << "\t";
	}
	cout << endl;
	if(!words.size()){
	    continue;
	}
	if(words[0] == "layout"){
	    if(words[1] == "parameters"){
		paramNo = atoi(words[2].c_str());
	    }
	    if(words[1] == "order"){
		for(int i=0; i < paramNo; i++){
		    order.push_back(words[2 + i]);
		}
	    }
	    if(words[1] == "sizes"){
		for(uint i=0; i < order.size(); i++){  /// ASSUMPTION, bits always comes before the others. (this is the rule of the format..)
		    if(order[i] == "bits"){
			bitNo = atoi(words[2 + i].c_str());
		    }
		    if(order[i] == "x"){    
			nx = atoi(words[2 + i].c_str());
			dims[i-1] = nx;
			t_dim[0] = i-1;    //
		    }
		    if(order[i] == "y"){
			ny = atoi(words[2 + i].c_str());
			dims[i-1] = ny;
			t_dim[1] = i-1;
		    }
		    if(order[i] == "z"){
			secNo = atoi(words[2 + i].c_str());
			dims[i-1] = secNo;
			t_dim[2] = i-1;
		    }
		    if(order[i] == "ch"){
			nw = (short)atoi(words[2+i].c_str());
			t_dim[3] = i-1;
			dims[i-1] = nw;
		    }
		    if(order[i] == "t"){
			nt = atoi(words[2+i].c_str());
			t_dim[4] = i-1;
			dims[i-1] = nt;
		    }
		}
	    }
	    if(words[1] == "significant_bits"){
		sigBits = atoi(words[2].c_str());
	    }
	}
	
	if(words[0] == "representation"){
	    if(words[1] == "format"){
		if(words[2] == "integer"){
		    switch(bitNo){
			case 8:
			    repFormat = 0;
			    break;
			case 16:
			    repFormat = 1;
			    break;
			case 32:
			    repFormat = 2;
			    break;
			default :
			    cerr << "unknown integer format with size : " << bitNo << endl;
			    exit(1);
		    }
		}
		if(words[2] == "float" || words[2] == "real"){   // assume bitNo = 32. 
		    repFormat = 3;
		}
	    }
	    if(words[1] == "sign"){
		if(words[2] == "signed"){
		    isSigned = true;
		}else{
		    isSigned = false;
		}
	    }
	    if(words[1] == "byte_order"){
		// assume that the byte order will either be big or small endian, and nothing stupid in
		// between. In this case all I need to do is to check if the first word is one or not..
		if(words[2] == "1"){
		    bigEnded = false;
		}
	    }
	    if(words[1] == "compression"){
		if(words[2] == "compressed"){
		    compressed = true;
		}
	    }
	}
	if(words[0] == "parameter"){
	    if(words[1] == "origin"){
		for(int i=0; i < paramNo; i++){
		    if(order[i] == "x"){
			x0 = atof(words[2 + i].c_str());
		    }
		    if(order[i] == "y"){
			y0 = atof(words[2 + i].c_str());
		    }
		    if(order[i] == "z"){
			z0 = atof(words[2 + i].c_str());
		    }
		}
	    }
	    if(words[1] == "scale"){
		// and again, go through the parameters.. and get what we want..
		for(int i=0; i < paramNo; i++){
		    int n = i + 2;
		    if(order[i] == "x"){
			dx = atof(words[n].c_str());
		    }
		    if(order[i] == "y"){
			dy = atof(words[n].c_str());
		    }
		    if(order[i] == "z"){
			dz = atof(words[n].c_str());
		    }
		}
	    }
	    // there's also the units, and labels, but I can't be bothered to deal with these at the moment..
	}
	if(words[0] == "sensor"){
	    if(words[1] == "s_params"){
		if(words[2] == "LambdaEm"){
		    // in which case there should be a word for each wave length.. or we are a bit screwed..
		    if(words.size() != (uint)(3 + nw)){
			cerr << "We don't appear to have enough channel information words size is " << words.size() << "  nw is : " << nw << endl;
			exit(1);
		    }
		    for(int i=0; i < nw; i++){
			waves[i] = atoi(words[3 + i].c_str());
		    }
		}
	    }
	    // there are also a range of other sensor paramters, but we don't need those just to display the image.. so.. 
	}
    }

    // at this point we should have all the crucial values set, such that we can create the data, and set the 
    // stuff up.. 
    cout << "dims are : " << nx << "  " << ny << "  " << secNo << "  " << nw << endl;
    cout << "scales   : " << dx << "  " << dy << "  " << dz << endl;
    cout << "scaled dims : " << float(nx) * dx << "  " << float(ny) * dy << "  " << float(secNo) * dz << endl;
    cout << "nw : " << nw << endl;
    for(int i=0; i < nw; i++){
	cout << "\t" << waves[i] << endl;
    }
    cout << "bigendian is " << bigEnded << endl;
    cout << "bitNo        " << bitNo << endl;
    cout << "sigBits      " << sigBits << endl;
    cout << "repFormat    " << repFormat << endl;
    
    // try and find the ids file..
    idsFile = idsFile.substr(0, idsFile.length() - 3);
    idsFile += "ids";
    ifstream in2(idsFile.c_str());
    if(!in2){
	cerr << "couldn't open the ids file. no data available. bummer " << endl;
	return(false);
    }
    cout << "opened " << idsFile << endl;
    // and then let's work out how to read this file.   Need to read in the proper order.. and convert the size to something small.
    // make the data structure that we want to use, and then simply work out how to address that data structure.. (i.e. a long thingy)
    // we are basically going to assume that the data is three dimensional for now.. 
    // bugger it, read the whole thing in. Then just work out a conversion matrix... and go through the bytes..
    // which is good as we can stick the whole thing through a byte order converter in one go.. need a big buffer but what the hell
    int voxelNo = nx * ny * nw * secNo;
    int frameSize = dims[0] * dims[1] * bitNo/8;
    int pixelNo = dims[0] * dims[1];   // the number of pixels per position.. 
    int bno = bitNo/8;
    sectionSize = nx * ny * bno;
    char* frame = new char[frameSize];
    char* swap = 0;
    float maxInt = pow(double(2), double(sigBits));   // the maximum intensity... -but if signed, then this is probably wrong.. 
//    float maxInt = powf(float(2), float(sigBits));   // the maximum intensity... -but if signed, then this is probably wrong.. 
    if(bigEnded){
	swap = new char[frameSize];
    }
    float* rgbData = new float[voxelNo];
    cout << "allocated rgbData .. total of " << voxelNo << "  floats " << endl;
    // but don't read all of it in, as it will take too much memory..
    cout << "reading in data in order : " << dims[4] << dims[3] << "\t" << dims[2] << "\t" << dims[1] << "\t" << dims[0] << endl;

    vector<float> maxValues(nw);
    for(uint i=0; i < maxValues.size(); i++){
	maxValues[i] = 0;
    }
//    float maxValue = 0;
    for(int h=0; h < dims[4]; h++){         
	int coord[5];
	coord[4] = h;
//	cout << "reading from dims 4 : " << h << endl;
	for(int i=0; i < dims[3]; i++){
//	    cout << "reading from dims 3 : " << i << endl;
//	    cout << i << endl;
	    coord[3] = i;
	    for(int j=0; j < dims[2]; j++){
		coord[2] = j;
//		cout << "reading from dims 2 " << j << endl;
		// read in a frame from the thingy. Check that the read number is good. 
		if(bigEnded){
		    in2.read(swap, frameSize);
		    swapBytes(swap, frame, dims[0] * dims[1], bitNo/8);
		}else{
		    in2.read(frame, frameSize);
		}
//		cout << "read into the buffer .. " << endl;
		// now we have to go through and put the values in the right place... 
		//cout << "\t" << j << "\t pix no: " << pixelNo << endl;
		for(int k=0; k < pixelNo; k++){
		    /// first determine where we are in this circumstance... 
		    coord[0] = k % dims[0];
		    coord[1] = k / dims[0];
		    int w = coord[t_dim[3]];
		    int x = coord[t_dim[0]];
		    int y = coord[t_dim[1]];
		    int z = coord[t_dim[2]];
		    //	    cout << "\t" << k << "\t" << w << "\t" << z << "\t" << y << "\t" << x << endl;
		    // the coords give the current coordinates in the read space, which are translated into the 
		    // the current coords in the write space (writing to memory.. ).
		    // from these we can calculate the position in the target rgbData..
		    //int n = w + x * nw + y * nw * nx + z * nw * nx * ny;
		    int n = x + y * nx + z * nx * ny + w * nx * ny * secNo;
		    // and then assign depending on the mode that we are using.. 
		    char* from = frame + k * bno;
		    switch(repFormat){
			case 0:   // char
			    rgbData[n] = (float)*from/ maxInt;
			    break;
			case 1:   // short
			    rgbData[n] = (float)*(short*)from/ maxInt;
			    break;
			case 2:
			    rgbData[n] = (float)*(int*)from/ maxInt;
			    break;
			case 3:
			    rgbData[n] = *(float*)from / (float)maxInt;
			    //    cout << rgbData[n] << "\t";
//			rgbData[n] = *(float*)from/ maxInt;
			    break;
			default :
			    cerr << "Unknown representation format : " << repFormat << endl;
			    exit(1);
		    }
		    if(maxValues[w] < rgbData[n]){ maxValues[w] = rgbData[n]; }
//		cout << "  " << rgbData[n];
		}
//	    cout << endl;
	    }
	}
	cout << "max value for channel : " << h << "  is : " << maxValues[h] << endl;

    }
//   cout << "\n\nMax Value is : " << maxValue << "\n\n" << endl;
    // scale all the values over maxValue..
    // this is bad, but will do for now..
    int volNo = secNo * ny * nx;
    for(int dw=0; dw < nw; dw++){
	for(int i=0; i < volNo; i++){
	    rgbData[dw * volNo + i] = rgbData[dw * volNo + i] / maxValues[dw];
	}
    }
    // which might work, you never know with these things... 
    // need to create the image Data and set the colours and stuff.. perhaps..
    vector<float> biasFactors(nw);
    vector<float> scaleFactors(nw);
    for(int i=0; i < nw; i++){
	biasFactors[i] = 0;
	scaleFactors[i] = 1;
    }
    delete frame;
    if(swap){
	delete swap;
    }
    // but I'm going to change this later on.. hmmmm.. 
    //    objects = new groupObject*[nx * ny * secNo];
    //memset(objects, 0, sizeof(groupObject*) * nx * ny * secNo);  // initialised to 0.

    //image = new imageData(rgbData, objects, nx, ny, secNo, nw, waves);

//    image = new imageData(rgbData, nx, ny, secNo, nw, waves, dx, dy, dz);

    setBiasAndScale(biasFactors, scaleFactors);
    cout << "set bias and scales " << endl;
    // and make the image object .. 
//    currentRGBImage = image->currentRGBImage();

//    return(false);   // needs rewriting.. 
    //exit(1);   
    return(true);
}


void DVReader::clearObjects(){
//   // first delete the objects..
//   for(int i=0; i < objectList.size(); i++){
//     delete objectList[i];
//   }
//   objectList.resize(0);
//   // and then change the points..
//   for(int z=0; z < secNo; z++){
//     for(int y=0; y < ny; y++){
//       for(int x=0; x < nx; x++){
// 	objects[z][y][x] = 0;
//       }
//     }
//   }
//   // and that should be it.. 
}

vector<float> DVReader::objectSums(int wl){
  // ugly function.. 
    vector<float> sums; wl = wl;  // ugly hack.. 
//   sums.reserve(objectList.size());
//   for(int i=0; i < objectList.size(); i++){
//     float s = objectList[i]->sum(wl);
//     if(s){
//       sums.push_back(s);
//     }
//   }
  return(sums);
}

DVReader::~DVReader(){
//    cout << "deleting image data.. " << endl;
//  scratch.close();
//  if(remove(scratchFile.c_str())){
//      cerr << "Unable to delete " << scratchFile << endl;
//  }
//  delete(image);

  //in->close();
  //delete imageData;
  //delete displayData;
}

int DVReader::nextSection(){
  currentSection++;
  if(currentSection >= secNo){
    currentSection = 0;
  }
  setImageData();
  return(currentSection);
}

void DVReader::setBiasAndScale(vector<float> bias, vector<float> scale){
    //image->setBiasAndScale(bias, scale);
//   if(bias.size() == biasFactors.size() && scale.size() == scaleFactors.size()){
//     biasFactors = bias;
//     scaleFactors = scale;
//     setImageData();   // which recalculates depending on the scale.. 
//   }else{
//     cerr << "Bias and Scale do not have the same sizes as scaleFactors.. and biasFactors " << endl;
//   }
  setImageData();
}

//void DVReader::setAdditive(vector<bool> addFactors){    
    //  image->setAdditive(addFactors);
//}

void DVReader::setImageData(){
  //  cout << endl << "beginning of setImageData nw " << nw << endl;
  if(currentSection >= secNo || currentSection < 0){
    return;
  }
//  currentRGBImage = image->setImage(currentSection);

}

float* DVReader::x_zSlice(int ypos){
        ypos = ypos; return(0);
//    return(image->set_x_zFrame(ypos));
}

// float* DVReader::y_zSlice(int xpos){
//     return(image->set_y_zFrame(xpos));
// }



int DVReader::previousSection(){
  currentSection--;
  if(currentSection < 0){
    currentSection = 0;
  }
  setImageData();
  return(currentSection);

}

bool DVReader::readBytes(int bn, float* row, ifstream& in){
  // assume there's enough space in row to read in the number of data points we are looking at
  short* shortRow = new short[bn];
  unsigned char* charRow = new unsigned char[bn];
  // no need to make a float as we can directly use the row above..
  bool ok = true;
  switch(mode){
  case 0:  // 1 byte unsigned...
    in.read((char*)charRow, bn);
    // no need to swap bytes, only 1 ..	
    break;
  case 1:  // 2 byte signed integer..
    in.read((char*)shortRow, bn * 2);
    if(!sameEndian){
	swapBytes((char*)shortRow, bn, 2);
    }
    break;
  case 2: // it's a float.. read straight into row..
    in.read((char*)row, bn * 4);
    if(!sameEndian){
	swapBytes((char*)row, bn, 4);
    }
    break;
  default :
    cerr << "can't cope with mode " << mode << endl;
    ok = false;
  }
  if(!ok){
    delete shortRow;
    delete charRow;
    return(false);
  }
  if(in.fail()){    
    cerr << "Failed somehow to read from the file, going back to the beginning,, to check where we are" << endl;
    in.clear();
    in.seekg(headerSize + (std::ios::pos_type)next);
    currentSection = 0;
    delete shortRow;
    delete charRow;
    return(false);
  }

  // so now we have to do another switch on the mode so we now which one has the data in it..
  switch(mode){
  case 0:
    for(int i=0; i < bn; i++){
      row[i] = (float)charRow[i] / (float)255;
    }
    break;
  case 1:
    for(int i=0; i < bn; i++){
      row[i] = (float)shortRow[i] / (float)maxSignal;
    }
    break;
  default :  // do nothing... if 2, then row has already been assigned anyway..
    // means it should be a float..
    ;
  }
  // if we are here, row has been assigned. ok is true.. and we should delete 
  // the temporary datastructures..
  delete charRow;
  delete shortRow;
  return(true);
}
    

void DVReader::setWaveColor(int wave, float r, float g, float b){
    //image->setColor(wave, r, g, b);
  setImageData();

}

void DVReader::useComponentFactors(bool on){
    //image->toggleUseComponentFactors(on);
    setImageData(); on = on;  // hack to reduce warnings.. 
}

// linearPeaks DVReader::findLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue){
//     return(image->findLocalMaxima(wl, radius, minPeakValue, maxEdgeValue));
// }

// void DVReader::findAllLocalMaxima(int wl, int radius, float minPeakValue, float maxEdgeValue, float bgm){
//     image->findAllMaxima(wl, radius, minPeakValue, maxEdgeValue, bgm);
    
//     // and let's also try with new function 
//     image->findAllPeaks(wl, radius, minPeakValue, maxEdgeValue, bgm);
// }

void DVReader::printInfo(){
  cout << "width :     " << nx << endl
       << "height:     " << ny << endl
       << "section no. " << nsec << endl
       << "channel no. " << nw << endl;
  for(int i=0; i < nw; i++){
    cout << "wave " << i + 1 << "  : " << waves[i] << "\tmin : " << mins[i] << "\tmax : " << maxs[i] << endl;
  }
  cout << "pixel size   : " << pSize << endl   // the number of bytes in a pixel.. 
       << "section Size : " << sectionSize << endl;
  
}


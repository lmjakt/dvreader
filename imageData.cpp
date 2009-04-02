#include "imageData.h"
#include "kcluster/kClusterProcess.h"
#include <iostream>
#include <time.h>
#include <math.h>
#include <wchar.h>
#include <algorithm>
#include <map>
#include <set>

using namespace std;

imageData::imageData(float* imData, int xn, int yn, int zn, int wn, int* wl, float x_s, float y_s, float z_s){
    useComponentFactors = false;
    data = imData;
//    in = scratch;
    //    data = dt;
    //    objects = gobjects;
    x = xn;
    y = yn;
    z = zn;
    w = wn;
    globalMargin = 30;      // though I don't know where to get this from, it seems normally to be the case.
    x_scale = x_s;
    y_scale = y_s;
    z_scale = z_s;
    wavelengths = wl;
    xOffsets = new int[w];
    yOffsets = new int[w];
    zOffsets = new int[w];
    displayChannel = new bool[w];
    additive = new bool[w];
    for(int i=0; i < w; i++){
	colorMappers.push_back(new ColorMapper());
	xOffsets[i] = 0;
	yOffsets[i] = 0;
	zOffsets[i] = 0;
	displayChannel[i] = true;
	additive[i] = true;
    }
    sliceMapper = new ColorMapper();
    currentFrameData = new float[w * x * y];   // then add a function which can destroy this and reallocated including merged colours
    frameBuffer = new float[w * x * y];
    mergedWaves = 0;
    mergedSizes = 0;
    mergedDefs = 0;   // defined these in an interactive function... 
    frame = new float[3 * x * y]; /// 3 colours per point RGB
    x_z_frame = new float[3 * x * z];
    x_z_FrameData = new float[w * x * z];
    x_z_FrameBuffer = new float[w * x * z];
    //   y_z_frame = new float[3 * y * z];   // slices in the opposite direction. (For us these are always going to be much smaller, but we'll need to scale them appropriately)
    //y_z_FrameData = new float[w * y * z];
    //y_z_FrameBuffer = new float[w * y * z];
    colors = new float[w * 3];
    biasFactors = new float[w];
    scaleFactors = new float[w];
    // lets initalise all of these to 0's to avoid any trouble..
    memset((void*)frame, 0, sizeof(float) * 3 * x * y);
    memset((void*)x_z_frame, 0, sizeof(float) * 3 * x * z);
    //memset((void*)y_z_frame, 0, sizeof(float) * 3 * y * z);
    memset((void*)colors, 0, sizeof(float) * w * 3);
    memset((void*)biasFactors, 0, sizeof(float) * w);
    memset((void*)scaleFactors, 0, sizeof(float) * w);
}

imageData::~imageData(){
    delete currentFrameData;
    delete frameBuffer;
    delete frame;
    delete data;
}

void imageData::setColor(int index, float r, float g, float b){
    // find if we have the wavelength..
    if(index >= 0 && index < (w + mergedWaves)){
	cout << "\t\tSETTING colors for some wl " << "  index is " << index << "  r: " << r << "  g: " << g << "  b: " << b << endl; 
	colors[index * 3] = r;
	colors[index * 3 + 1] = g;
	colors[index * 3 + 2] = b;
	displayChannel[index] = ( (r + g + b) > 0 );   // if set to black don't display
    }else{
	cerr << "couldn't get index for set color bugger " <<endl;
    }
//    for(int i=0; i < (w + mergedWaves); i++){
//	cout << "\t" << i << "\tr: " << colors[i*3] << "\tg: " << colors[i*3 + 1] << "\tb: " << colors[i*3 + 2] << endl;
//    }
}

void imageData::setOffsets(int index, int xo, int yo, int zo){
    // only allow positive offsets ..
    cout << "imageData.cpp setOffsets index " << index << "  xo: " << xo << "  yo: " << yo << "  zo: " << zo << endl;
    if(xo < 0 || yo < 0){
	cerr << "negative offset don't like that" << endl;
	return;
    }
    if(index >=0 && index < w){
	xOffsets[index] = xo;
	yOffsets[index] = yo;
	zOffsets[index] = zo;
	cout << "\t\toffsets changed " << endl;
    }
    // which is everything we need to do..
}

void imageData::setBiasAndScale(vector<float> bias, vector<float> scale){
    if(bias.size() == scale.size() && bias.size() == (uint)(w + mergedWaves)){
	for(int i=0; i < (w + mergedWaves); i++){
	    biasFactors[i] = bias[i];
	    scaleFactors[i] = scale[i];
	    cout << "i : "  << i << "  bias : " << biasFactors[i] << "  scale : " << scaleFactors[i] << endl;
	}
    }else{
	cerr << "Bias and Scale not set appropriately " << endl;
    }
    cout << "end of setBias and scale, withouth any apparent problems " << endl;
}

void imageData::setAdditive(vector<bool> addFactors){
    if(addFactors.size() != (uint)(w + mergedWaves)){
	cerr << "setAdditive wrong size of addFactors is : " << addFactors.size() << "\t but should be " << w + mergedWaves <<endl;
	return;
    }
    for(uint i=0; i < addFactors.size(); i++){
	additive[i] = addFactors[i];
    }
    // leave it to the caller to determine if we should redisplay at this point..
}

void imageData::addMergeChannel(int* mergeI, int s){
    // first check the sanity of s.
    if(s >= w){
	cerr << "addMergeChannel s is larger than w" << endl;
	return;
    }
    for(int i=0; i < s; i++){
	if(mergeI[i] >= w){
	    cerr << "addMergeChannel, merge[" << i << "] = " << mergeI[i] << " is larger than " << w << endl;
	    return;
	}
    }
    int mcount = s;     // remnant of worse code.. 
    
    /// so now we want to add a merge channel to the struture..
    int** tempMerged = new int*[mergedWaves + 1];
    int* tempSizes = new int[mergedWaves + 1];
    for(int i=0; i < mergedWaves; i++){
	tempMerged[i] = new int[mergedSizes[i]];
	tempSizes[i] = mergedSizes[i];
	for(int j=0; j < mergedSizes[i]; j++){
	    tempMerged[i][j] = mergedDefs[i][j];
	}
    }
    // then define the new size as mcount above..
    tempSizes[mergedWaves] = mcount;
    tempMerged[mergedWaves] = new int[mcount];
    for(int i=0; i < mcount; i++){
	tempMerged[mergedWaves][i] = mergeI[i];
    }
    // which should be ok..
    // now just delete and reassign..
    for(int i=0; i < mergedWaves; i++){ delete mergedDefs[i]; }
    delete mergedDefs;
    delete mergedSizes;
    mergedDefs = tempMerged;
    mergedSizes = tempSizes;
    mergedWaves++;
    // we also need to grow biasFactors and the scaleFactors. need to consider the colour allocation, presumably best done by
    // and the colours as well as the currentFrameData vector...
    float* tempColors = new float[3 * (w + mergedWaves)];
    for(int i=0; i < 3 * (w + mergedWaves - 1); i++){
	//    for(int i=0; i < 3 * (3 + mergedWaves -1 ); i++){
	tempColors[i] = colors[i];
    }
    // leave the rest undefined.. random color... !! until set manually.. 
    bool *tempDisplayChannel = new bool[w + mergedWaves];
    bool* tempAdditive = new bool[w + mergedWaves];
    float* tempBias = new float[w + mergedWaves];
    float* tempScale = new float[w + mergedWaves];
    for(int i=0; i < (w + mergedWaves - 1); i++){
	tempBias[i] = biasFactors[i];
	tempScale[i] = scaleFactors[i];
	tempDisplayChannel[i] = displayChannel[i];
	tempAdditive[i] = additive[i];
    }
    tempBias[w + mergedWaves -1] = 0;
    tempScale[w + mergedWaves -1] = 1;    // dear god that is ugly...
    tempDisplayChannel[w + mergedWaves -1] = true;
    tempAdditive[w + mergedWaves - 1] = true;
    // then we need to 
    delete colors;
    delete biasFactors;
    delete scaleFactors;
    delete displayChannel;
    delete additive;
    colors = tempColors;
    biasFactors = tempBias;
    scaleFactors = tempScale;
    displayChannel = tempDisplayChannel;
    additive = tempAdditive;
    /// whooah, that is ugly... 
    delete currentFrameData;
    currentFrameData = new float[x * y * (w + mergedWaves)];
    // and do the same for the slices..
    delete x_z_FrameData;
    x_z_FrameData = new float[x * z * (w + mergedWaves)];
    //delete y_z_FrameData;
    //y_z_FrameData = new float[y * z * (w + mergedWaves)];
    //
    colorMappers.push_back(new ColorMapper());
}

void imageData::readFrame(int frameNo, int waveIndex, int x_begin, int y_begin, int width, int height, float* dest){
    // unfortunately it is difficult to actually just read this in one go.. we have to read into a seperate buffer first
    // or we have to read a line at a time. 
    
    // Read all the lines necessary for the given thing..
//    float* buffer = new float[height * x];   // i.e. the rows, but not the other stuff..
    if(frameNo >= z || frameNo < 0 || waveIndex >= w || waveIndex < 0){
	cerr << "imageData::readFrame inappropriate frameNo or waveIndex : " << frameNo << "\t" << waveIndex << endl;
	return;
    }

    float* buffer = data + waveIndex * z * x * y + frameNo * x * y + y_begin * x;
    
//    (*in).seekg(sizeof(float) * (waveIndex * z * x * y  + frameNo * x * y + y_begin * x) );
//    (*in).read( (char*)buffer, sizeof(float) * x * height);

    // and then we have to copy over the relative data into the appropriate place.
    // The quickest way to do this is probably to use memcpy... 
    for(int i=0; i < height; i++){
	memcpy(dest + i * width, buffer + i * x + x_begin, sizeof(float) * width);
    }
//    delete buffer;
    // and we are done.. but horrible function that does not check the parameters.. aiiayaa..
    // but at least its a private function.. 
}

//void imageData::readFrame(int frameNo, int waveIndex, float* dest){
float* imageData::readFrame(int frameNo, int waveIndex){
    // first check that the frame and waveIndex numbers are ok..
    if(frameNo >= z || frameNo < 0 || waveIndex >= w || waveIndex < 0){
	cerr << "imageData::readFrame inappropriate frameNo or waveIndex : " << frameNo << "\t" << waveIndex << endl;
	return(data);    // bad, but at least something.. 
    }
    // change this so that we don't have to do a mem
    return(data + waveIndex * z * y * x + frameNo * y * x);

//    (*in).seekg(sizeof(float) * (waveIndex * z * x * y  + frameNo * x * y));
//    (*in).read( (char*)dest, sizeof(float) * x * y);
    // which is about all we need to do..
}

void imageData::paintVolumePeaks(threeDPeaks& peaks, float* dest, int frameNo){
    // peaks are references in global coordinates so it is necessary to translate these back. But that is pretty easy..

//    int fs = x * y;

//    int bpos = frameNo * fs;
    //   int epos = (frameNo + 1) * fs;
    // don't assume that the peaks are sorted so do go through all of them. Sorting them would
    // take much longer than just going through them
    cout << "paintVolumePeaks : peaks.size : " << peaks.peaks.size() << "\tdrops size " << peaks.drops.size() << endl;

    // also paint all the linearPeaks.. (temporary, to work out what's going wrong.
     map<long, linearPeak>::iterator lit;
//     for(lit = peaks.xPeaks.begin(); lit != peaks.xPeaks.end(); lit++){
// 	if((*lit).second.position >= bpos && (*lit).second.position < epos){
// 	    int j = 3 * ((*lit).second.position - bpos);
// 	    dest[j] = 1.0;
// 	}
//     }
//     for(lit = peaks.yPeaks.begin(); lit != peaks.yPeaks.end(); lit++){
// 	if((*lit).second.position >= bpos && (*lit).second.position < epos){
// 	    int j = 3 * ((*lit).second.position - bpos);
// 	    dest[j+1] = 1.0;
// 	}
//     }

//     for(lit = peaks.zPeaks.begin(); lit != peaks.zPeaks.end(); lit++){
// 	if((*lit).second.position >= bpos && (*lit).second.position < epos){
// 	    int j = 3 * ((*lit).second.position - bpos);
// 	    dest[j+2] = 1.0;
// 	}
//     }

    map<long, simple_drop>::iterator it;
    for(it = peaks.simpleDrops.begin(); it != peaks.simpleDrops.end(); it++){
	if((*it).second.z == frameNo){
	    int j = 3 * ((*it).second.y * x + (*it).second.x);
	    if((*it).second.sumValue >= peaks.minThreshold && (*it).second.sumValue <= peaks.maxThreshold){
		dest[j] = 0.6;
		dest[++j] = 0.6;
		dest[++j] = 0.6;
	    }else{
		dest[j] = 0.7;
		dest[j+1] = 0.0;
		dest[j+2] = 0.7;
	    }
	}
    }

//     for(uint i=0; i < peaks.drops.size(); i++){
// //    for(uint i=0; i < peaks.peaks.size(); i++){
// 	if(peaks.drops[i].center >= bpos && peaks.drops[i].center < epos){
// 	    int j = 3 * (peaks.peaks[i] - bpos);
// 	    if(peaks.drops[i].totalValue >= peaks.minThreshold && peaks.drops[i].totalValue <= peaks.maxThreshold){
// 		dest[j] += peaks.r;
// 		dest[++j] += peaks.g;
// 		dest[++j] += peaks.b;
// 	    }else{
// 		dest[j] = 0.0;
// 		dest[++j] = 1.0;
// 		dest[++j] = 0.0;
// 	    }
// 	}
//     }
}

void imageData::paintPerimeter(int margin, float* dest){
    // first draw two horizontal lines..
    if(margin < y){
	for(int i=margin; i < x-margin; i++){
	    dest[3 * (margin * x + i)] = 1.0;
	    dest[3 * (margin * x + i) + 1] = 1.0;
	    dest[3 * (margin * x + i) + 2] = 1.0;
	    // and the bottom line.. 
	    dest[3 * ((y - margin) * x + i)] = 1.0;
	    dest[3 * ((y - margin) * x + i) + 1] = 1.0;
	    dest[3 * ((y - margin) * x + i) + 2] = 1.0;
	}
    }
    // then two vertical lines..
    if(margin < x){
	for(int i=margin; i < y-margin; i++){
	    dest[3 * (i * x + margin)] = 1.0;
	    dest[3 * (i * x + margin) + 1] = 1.0;
	    dest[3 * (i * x + margin) + 2] = 1.0;
	    // and the right line.. 
	    dest[3 * ((1 + i) * x - margin)] = 1.0;
	    dest[3 * ((1 + i) * x - margin) + 1] = 1.0;
	    dest[3 * ((1 + i) * x - margin) + 2] = 1.0;
	}
    }
}

void imageData::paintNuclearLines(float* dest){
    // if we have any nuclar lines then draw them on the currently thingy..
//     for(uint i=0; i < nuclearLines.size(); i++){
// 	for(int p = nuclearLines[i].start; p <= nuclearLines[i].stop; p++){
// 	    dest[3 * p + 1] = 1.0;
// 	    dest[3 * p + 2] = 1.0;
// 	}
//     }
    // if we have any perimeters then go through those...
    for(uint i=0; i < nuclei.size(); i++){
//    for(uint i=0; i < nuclearPerimeters.size(); i++){
	for(uint j=0; j < nuclei[i].perimeter.size(); j++){
//	for(uint j=0; j < nuclearPerimeters[i].size(); j++){
	    dest[3 * (nuclei[i].perimeter[j].y * x + nuclei[i].perimeter[j].x)] = 1.0;
	    dest[3 * (nuclei[i].perimeter[j].y * x + nuclei[i].perimeter[j].x) + 1] = 1.0;
	    dest[3 * (nuclei[i].perimeter[j].y * x + nuclei[i].perimeter[j].x) + 2] = 1.0;
	}
	for(uint j=0; j < nuclei[i].smoothPerimeter.size(); j++){
//	for(uint j=0; j < nuclearPerimeters[i].size(); j++){
	    dest[3 * (nuclei[i].smoothPerimeter[j].y * x + nuclei[i].smoothPerimeter[j].x)] = 1.0;
	    dest[3 * (nuclei[i].smoothPerimeter[j].y * x + nuclei[i].smoothPerimeter[j].x) + 1] = 1.0;
	    dest[3 * (nuclei[i].smoothPerimeter[j].y * x + nuclei[i].smoothPerimeter[j].x) + 2] = .0;
	}

	
    }
}

float* imageData::setImage(int frameNo){
//    timespec time;
    if(frameNo < 0 || frameNo >= z){
	cerr << "frameNo out of bounds " << frameNo << endl;
	return(frame);
    }
    // seek in scratch file to get to the start of the current frame..
    int zero = 0;
    for(int i=0; i < w; i++){
	int fn = frameNo + zOffsets[i];
	if(fn < 0 || fn >= z){
	    memset((void*)(frameBuffer + (i * x * y)), zero, x * y * sizeof(float));
	}else{
	    memcpy(frameBuffer + i * x * y, data + i * x * y * z + fn * x * y, sizeof(float) * x * y);
//	    (*in).seekg(sizeof(float) * (i * z * x * y +  fn * x * y));
//	    (*in).read( (char*) (frameBuffer + (i * x * y) ), sizeof(float) * x * y);   // the number of bytes and everything... 
	}
    }
//    if(in->fail()){
//	cerr << "unable to read from scratch file " << endl;
//	exit(1);
//    }
    // first we need to set the destination memory area to 0.
    memset((void*)frame, zero, x * y * 3 * sizeof(float));
    memset((void*)currentFrameData, zero, x * y * (w + mergedWaves) * sizeof(float));   // as if we have offsets we have to allow for these things.. 

    // rather than doing everything here, let's call the apropriate functions in each of the color mappers, and let them 
    // get on with stuff...
    for(int i=0; i < w; i++){      // first for the single colours.. which is kind of simple.. 
	colorMappers[i]->mapSingle(frameBuffer + i * x * y, x, y, xOffsets[i], yOffsets[i], frame, currentFrameData + i, w + mergedWaves, 
				       colors + i * 3, biasFactors + i, scaleFactors + i, additive[i]);
   }
    // and then for the merged waves... -- just use the single function first for checking..
    for(int k=0; k < mergedWaves; k++){
	int i = k + w;
//	if(displayChannel[i]){
	if(useComponentFactors){
	    colorMappers[i]->mapMergedIndividualComponents(frameBuffer, x, y, xOffsets, yOffsets, frame, currentFrameData + i, w + mergedWaves, 
							   colors + i * 3, biasFactors, scaleFactors, mergedDefs[k], mergedSizes[k], additive[i]);
	}else{
	    colorMappers[i]->mapMerged(frameBuffer, x, y, xOffsets, yOffsets, frame, currentFrameData + i, w + mergedWaves, 
				       colors + i * 3, biasFactors + i, scaleFactors + i, mergedDefs[k], mergedSizes[k], additive[i]);
	}
//	}
    }

//     clockRes = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
//     long mappingStarted = time.tv_nsec;

    // make sure that the color mappers are done before proceeding..
     for(uint i=0; i < colorMappers.size(); i++){
 	colorMappers[i]->wait();
     }
     
//     clockRes = clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time);
//     long mappingEnd = time.tv_nsec;
//     cout << "start Time : " << startTime << "\t" << startTime - startTime <<  endl
// 	 << "read Start : " << readStart << "\t" << readStart - startTime << "\t" << (readStart - startTime)/1e6 << endl
// 	 << "read End   : " << readEnd   << "\t" << readEnd - readStart << "\t" << (readEnd - readStart)/1e6 << endl
// 	 << "map begin  : " << mappingStart << "\t" << mappingStart - readEnd << "\t" << (mappingStart - readEnd)/1e6 << endl
// 	 << "map started: " << mappingStarted << "\t" << mappingStarted - mappingStart << "\t" << (mappingStarted - mappingStart)/1e6 << endl
// 	 << "map end    : " << mappingEnd << "\t" << mappingEnd - mappingStarted << "\t" << (mappingEnd - mappingStarted)/1e6  << endl;
     // And at this point if we have any models to fit then let's go through those and fit the data...
     if(useComponentFactors){
	 map<int, threeDPeaks>::iterator it;
	 for(it = volumePeaks.begin(); it != volumePeaks.end(); it++){
	     paintVolumePeaks((*it).second, frame, frameNo);
	 }
     }

     bool paint_perimeter = true;
     if(paint_perimeter){
	 paintPerimeter(globalMargin, frame);
     }

     bool paint_nuclearLines = true;
     if(paint_nuclearLines){
	 paintNuclearLines(frame);
     }

    return(frame);
}

float* imageData::set_x_zFrame(unsigned int ypos){
    // if ypos is smaller than just return the pointer without changing it..
    if(ypos >= (uint)y){
	cerr << "Can't assign x-z frame to ypos of : " << ypos << endl;
	return(x_z_frame);
    }
    // I don't think we can just move a pointer along here, as we have to define the positions in
    // a different order. Still, we just have to do a multiplication, and some addition, and there aren't so
    // many pixels..

//     float* source = new float[w];
//     int fsize = sizeof(float);
//     for(int dz=0; dz < z; dz++){
// 	for(int dx=0; dx < x; dx++){
// 	    float* dest = x_z_frame + dz * x * 3 + dx * 3;
// 	    dest[0] = dest[1] = dest[2] = 0;
// 	    (*in).seekg(fsize * (ypos * x * w + dz * y * x * w + dx * w));
// 	    (*in).read((char*)source, fsize * w); 
// 	    //	float* source = data + ypos * x * w + dz * y * x * w + dx * w;   // I think..
// 	    for(int k=0; k < w; k++){
// 		float v = source[k] * scaleFactors[k] + biasFactors[k];
// 		dest[0] += v * colors[k * 3];
// 		dest[1] += v * colors[k * 3 + 1];
// 		dest[2] += v * colors[k * 3 + 2];
// 	    }
// 	}
//     }
//     delete source;
//     return(x_z_frame);


//     float* source = new float[x];
//     int fsize = sizeof(float);
//     int zero = 0;
//     memset((void*)x_z_frame, zero, fsize * z * x * 3);   // I think.. 
//     for(int i=0; i < w; i++){
// 	for(int dz=0; dz < z; dz++){
// 	    float* dest = x_z_frame + dz * x * 3;
// 	    (*in).seekg(fsize * (i * x * y * z + dz * x * y + ypos * x));
// 	    (*in).read((char*)source, fsize * x);
// 	    // and then we have to go through each and check the value for each one
// 	    for(int dx=0; dx < x; dx++){
// 		float v = source[dx] * scaleFactors[i] + biasFactors[i];
// 		if(v > 0){
// 		    dest[0] += v * colors[i * 3];
// 		    dest[1] += v * colors[i * 3 + 1];
// 		    dest[2] += v * colors[i * 3 + 2];
// 		}
// 		dest += 3;
// 	    }
// 	}
//     }
//     delete source;
//     return(x_z_frame);


    // or using a ColorMapper .. we can do it this way.. 
    int fsize = sizeof(float);
    // pretend we are not using any offsets..
    // as these would have to be different..
    // int* offsets = new int[w];
    //   memset((void*)offsets, 0, w * sizeof(int));
    memset((void*)x_z_frame, 0, x * z * 3 * sizeof(float));
    // first fill the frame buffer..
    // It might seem silly to do fill in a buffer for all the colours, but this should simplify using
    // the color mapper to do merged colours. Eventually this might work out to be a very useful thing.
    for(int i=0; i < w; i++){
	int dy = ypos + yOffsets[i];
	if(dy < 0 || dy >= y){
	    memset((void*)(x_z_FrameBuffer + i * x * z), 0, x * z * fsize);
	}else{ 
	    for(int dz=0; dz < z; dz++){
		memcpy(x_z_FrameBuffer + i * x * z + dz * x, data + i * x * y * z + dz * x * y + dy * x, fsize * x);
//		(*in).seekg(fsize * (i * x * y * z + dz * x * y + dy * x));
//		(*in).read((char*)(x_z_FrameBuffer + i * x * z + dz * x), fsize * x);   // reads one row to the approriate location..
	    }
	}
    }
    // then go through the individual wavelengths, and merged ones calling the appropriate numbers..
    for(int i=0; i < w; i++){
	sliceMapper->mapSingle(x_z_FrameBuffer + i * x * z, x, z, xOffsets[i], zOffsets[i], x_z_frame, x_z_FrameData + i, w + mergedWaves,
			      colors + i * 3, biasFactors + i, scaleFactors + i);
//	x_z_mapper->mapSingle(x_z_FrameBuffer + i * x * z, x, z, 0, 0, x_z_frame, x_z_FrameData + i, w + mergedWaves,
//			      colors + i * 3, biasFactors + i, scaleFactors + i);
	sliceMapper->wait();
    }
    // and then the merged colours..
    for(int k=0; k < mergedWaves; k++){
	int i = k + w;
	if(useComponentFactors){
	    sliceMapper->mapMergedIndividualComponents(x_z_FrameBuffer, x, z, xOffsets, zOffsets, x_z_frame, x_z_FrameData + i, w + mergedWaves,
						      colors + i * 3, biasFactors, scaleFactors, mergedDefs[k], mergedSizes[k]);
	}else{
	    sliceMapper->mapMerged(x_z_FrameBuffer, x, z, xOffsets, zOffsets, x_z_frame, x_z_FrameData + i, w + mergedWaves,
				  colors + i * 3, biasFactors + i, scaleFactors + i, mergedDefs[k], mergedSizes[k]);
	}
	sliceMapper->wait();
    }
//    delete offsets;
    return(x_z_frame);
} 

// float* imageData::set_y_zFrame(unsigned int xpos){
// //     if(xpos >= x){
// // 	cerr << "Can't assign y-z frame to xpos of : " << xpos << endl;
// // 	return(y_z_frame);
// //     }
// //     float* source = new float[w];
// //     int fsize = sizeof(float);
// //     for(int dz=0; dz < z; dz++){
// // 	for(int dy=0; dy < y; dy++){
// // 	    float* dest = y_z_frame + dz * y * 3 + dy * 3;
// // 	    dest[0] = dest[1] = dest[2] = 0;
// // 	    (*in).seekg(fsize * (xpos * w + dz * x * y * w + dy * x * w));
// // 	    (*in).read((char*)source, fsize * w);
// // 	    //	float* source = data + xpos * w + dz * x * y * w + dy * x * w;
// // 	    for(int k=0; k < w; k++){
// // 		float v = source[k] * scaleFactors[k] + biasFactors[k];
// // 		dest[0] += v * colors[k * 3];
// // 		dest[1] += v * colors[k * 3 + 1];
// // 		dest[2] += v * colors[k * 3 + 2];    
// // 	    }
// // 	}
// //     }
// //     return(y_z_frame);


//     // or using a ColorMapper .. we can do it this way.. 
// //     int fsize = sizeof(float);
// //     // pretend we are not using any offsets..
// //     // as these would have to be different..
// //     //int* offsets = new int[w];
// //     //memset((void*)offsets, 0, w * sizeof(int));
// //     memset((void*)y_z_frame, 0, y * z * 3 * sizeof(float));
// //     // first fill the frame buffer..
// //     // It might seem silly to do fill in a buffer for all the colours, but this should simplify using
// //     // the color mapper to do merged colours. Eventually this might work out to be a very useful thing.
// //     for(int i=0; i < w; i++){
// // 	int dx = xpos + xOffsets[i];
// // 	if(dx < 0 || dx >= x){
// // //	    memset((void*)(y_z_FrameBuffer + i * y * z), 0, y * z * fsize);  // not necessary it's already set to zero.. above
// // 	    cerr << "value of dx is not good for setting the slice for y_z slice " << endl;
// // 	}else{ 
// // 	    for(int dz=0; dz < z; dz++){
// // 		for(int dy=0; dy < y; dy++){
// // 		    (*in).seekg(fsize * (i * x * y * z + dz * x * y + dy * x + dx));
// // 		    (*in).read((char*)(y_z_FrameBuffer + i * x * z + dz * x + dx), fsize);   // reads one row to the approriate location..
// // 		}
// // 	    }
// // 	}
// //     }
// //     // then go through the individual wavelengths, and merged ones calling the appropriate numbers..
// //     for(int i=0; i < w; i++){
// // 	sliceMapper->mapSingle(y_z_FrameBuffer + i * y * z, y, z, yOffsets[i], zOffsets[i], y_z_frame, y_z_FrameData + i, w + mergedWaves,
// // 			      colors + i * 3, biasFactors + i, scaleFactors + i);
// // //	x_z_mapper->mapSingle(x_z_FrameBuffer + i * x * z, x, z, 0, 0, x_z_frame, x_z_FrameData + i, w + mergedWaves,
// // //			      colors + i * 3, biasFactors + i, scaleFactors + i);
// // 	sliceMapper->wait();
// //     }
// //     // and then the merged colours..
// //     for(int k=0; k < mergedWaves; k++){
// // 	int i = k + w;
// // 	if(useComponentFactors){
// // 	    sliceMapper->mapMergedIndividualComponents(y_z_FrameBuffer, y, z, yOffsets, zOffsets, y_z_frame, y_z_FrameData + i, w + mergedWaves,
// // 						      colors + i * 3, biasFactors, scaleFactors, mergedDefs[k], mergedSizes[k]);
// // 	}else{
// // 	    sliceMapper->mapMerged(y_z_FrameBuffer, y, z, yOffsets, zOffsets, y_z_frame, y_z_FrameData + i, w + mergedWaves,
// // 				  colors + i * 3, biasFactors + i, scaleFactors + i, mergedDefs[k], mergedSizes[k]);
// // 	}
// // 	sliceMapper->wait();
// //     }
// //     //delete offsets;
// //     return(y_z_frame);
//     return(0);

// }
float* imageData::frameData(int frameNo){
    frameNo = frameNo;  // to get rid of annoying warning sign.. 
    // actually just returns the current one. Ignores the frameNo
    return(currentFrameData);
}
int imageData::channelNo(){
    return(w + mergedWaves);
}

float* imageData::currentRGBImage(){
    return(frame);
}

void imageData::toggleUseComponentFactors(bool on){
    useComponentFactors = on;
}

vector<vector<float> > imageData::xLine(int ypos){
    //cout << "imageData::xLine ypos : " << ypos << endl;
    vector<vector<float> > values(w + mergedWaves);
    for(uint i=0; i < values.size(); i++){
	values[i].resize(x);
    }
    if(ypos >= y || ypos < 0){
	cerr << "imageData::yLine inappropriate ypos: " << ypos << endl;
	return(values);
    }
    // otherwise, fairly simple, just go through the values in thingy and do stuff..
    float* source = currentFrameData + ypos * x * (w + mergedWaves);
    for(int i=0; i < x; i++){
	for(uint j=0; j < values.size(); j++){
	    values[j][i] = *source;
	    source++;
	}
    }
    return(values);
}

vector<vector<float> > imageData::yLine(int xpos){
    vector<vector<float> > values(w + mergedWaves);
    for(uint i=0; i < values.size(); i++){
	values[i].resize(y);
    }
    if(xpos >= x || xpos < 0){
	cerr << "imageData::yLine inappropriate xpos: " << xpos << endl;
	return(values);
    }
    // this is a bit more complicated as we have to calculate the offset for each position
    // but not that bad really..
    int cno = w + mergedWaves;   // the total number of merged waves... 
    for(uint i=0; i < values.size(); i++){
	for(int j=0; j < y; j++){
	    values[i][j] = currentFrameData[j * x * cno + xpos * cno + i];
	}
    }
    return(values);
}

vector<voxel> imageData::findPeaks(float* line, int length, int pr, float minPeakValue, float maxEdgeValue, bool pts){
    vector<voxel> peaks;
    int i=0;
    float mev;   // mev = peak value multiplied by maxEdgeValue (which is the proportion of the peak value..)
    while(i < length-1){
//	cout << i << ", ";
	int br = 0;          // before radius
//	int ps = i;          // peak start
	while(i < length-1 && line[i+1] >= line[i]){
	    if(pts){
		cout << "\t | "<< i << "\t" << line[i] << " --> " << line[i+1];
	    }
	    br++;
	    i++;
	}
	if(pts){
	    cout << endl;
	}
	// peak reached.. if peak satisfies certain criteria then go on and find the next trough..
	if(i >= length-1){           // i.e. we've stopped because we reached the end. (Stopping here means that we can miss an end peak, but I'm going to ignore that for now
	    if(pts){
		cout << "\t at end of line i : " << i << endl;
	    }
	    break;
	}
	// then check for the values..
	// the peak value should be at least minPeakValue
	if(line[i] < minPeakValue){
	    if(pts){
		cout << "peak at : " << i << " is less than minPeakValue : " << line[i] << " < " << minPeakValue << endl;
	    }
	    i++;
	    continue;
	}
	int pp = i;
	mev = maxEdgeValue * line[pp];
	
	// if br is larger than pr then check the value at i-pr, otherwise check at i-br..
	if(br > pr && line[i-pr] > mev){
	    if(pts){
		cout << "peak at : " << i << " but br > pr " << br << " > " << pr << "  and line[" << i-pr << "] is larger than " << mev << endl;
	    }
	    i++;
	    continue;
	}
	if(br <= pr && line[i-br] > mev){
	    if(pts){
		cout << "peak at : " << i << " but br <= pr : " << br << " <= " << pr << "  and line[" << i-br << "] : " << line[i-br] << " is larger than " << mev << endl;
	    }
	    i++;
	    continue;
	}
	// this peak seems to be fine. Now we have to find the end of the peak.. 
	int ar = 0;   // after radius..
	if(pts){
	    cout << "Found ok peak at : " << pp << endl;
	}
	while(i < length-1 && line[i+1] <= line[i]){
	    if(pts){
		cout << "\t | " << i << "\t" << line[i] << " --> " << line[i+1] << endl;
	    }
	    i++;
	    ar++;
	}
	if(pts){
	    cout << endl;
	}
	if(i >= length-1){   // reached the end of the line.. 
	    if(pts){
		cout << "at end of line i : " << i << endl;
	    }
	    break;
	}
	if(ar > pr && line[pp+pr] > mev){
	    if(pts){
		cout << "ar > pr : " << ar << " > " << pr << "  and line[" << pp + pr << "] : " << line[pp + pr] << "  is larger than " << mev << endl;
	    }
//	    i++;
	    continue;
	}
	if(ar <= pr && line[pp+ar] > mev){
	    if(pts){
		cout << "ar <= pr : " << ar << " <= " << pr << "  and line[" << pp + ar << "] : " << line[pp+ar] << "  is larger than " << mev << endl;
	    }
//	    i++;
	    continue;
	}
	// and if here we actually do have a peak..
	peaks.push_back(voxel(pp, line[pp]));
	if(pts){
	    cout << "pushed back peak to : " << peaks.size() << endl;
	}
//	i++;
    }
    return(peaks);
}

vector<linearPeak> imageData::findPeaks(float* line, int length, int pr, float minPeakValue, float maxEdgeValue, int dim){
    vector<linearPeak> peaks;
    int i=0;
    float mev;   // mev = peak value multiplied by maxEdgeValue (which is the proportion of the peak value..)
    while(i < length-1){
//	cout << i << ", ";
	int br = 0;          // before radius
	int ps = i;          // peak start
	while(i < length-1 && line[i+1] >= line[i]){
	    br++;
	    i++;
	}
	// peak reached.. if peak satisfies certain criteria then go on and find the next trough..
	if(i >= length-1){           // i.e. we've stopped because we reached the end. (Stopping here means that we can miss an end peak, but I'm going to ignore that for now
	    break;
	}
	// then check for the values..
	// the peak value should be at least minPeakValue
	if(line[i] < minPeakValue){
	    i++;
	    continue;
	}
	int pp = i;
	mev = maxEdgeValue * line[pp];
	
	// if br is larger than pr then check the value at i-pr, otherwise check at i-br..
	if(br > pr && line[i-pr] > mev){
	    i++;
	    continue;
	}
	if(br <= pr && line[i-br] > mev){
	    i++;
	    continue;
	}
	// this peak seems to be fine. Now we have to find the end of the peak.. 
	int ar = 0;   // after radius..
	while(i < length-1 && line[i+1] <= line[i]){
	    i++;
	    ar++;
	}
	if(i >= length-1){   // reached the end of the line.. 
	    break;
	}
	if(ar > pr && line[pp+pr] > mev){
	    continue;
	}
	if(ar <= pr && line[pp+ar] > mev){
	    continue;
	}
	int pe = pr > ar ? pp + ar : pp + pr;
	ps = br > pr ? pp - pr : ps;
	// and if here we actually do have a peak..
	peaks.push_back(linearPeak(pp, ps, pe, dim, pe-ps, line[pp], line[ps], line[pe]));
    }
    return(peaks);
}

linearPeaks imageData::findLocalMaxima(int wl, int pr, float minPeakValue, float maxEdgeValue){
    cout << "imageData::findLocalMaxima .." << wl << "\t" << pr << "\t" << minPeakValue << "\t" << maxEdgeValue << endl;
    // this is an optimised peak searching algorithm.
    //
    // 1. Look for linear peaks in rows and remember those
    // 2. Look for linear peaks in columns and remember those
    // 3. Pixels which are peaks in both rows and columns are considered two dimensional peaks.
    // 
    
    // For now we can only handle raw data. That is to say that we are not going to bother trying to deal with
    // merged channels. The reason for this is because of the way in which data is formatted (which makes for ugly
    // semantics). However, we should be able to change this in the future.. 
    linearPeaks peaks;
    if(wl > w){
	cerr << "wl " << wl << " is larger than w " << w << endl;
	return(peaks);
    }
    
    //float* xbuffer = new float[x];   // put x values into here  -- not necessary just pass a pointer... 
    float* ybuffer = new float[y];   // and y values here..

    float* source = frameBuffer + wl * x * y;
    // First find in rows (this is easy...)...
    int dy;   // we may use this later for finding in columns..
    int dx;
    vector<voxel> rowPeaks;
    bool printStuff = false;
    for(dy=0; dy < y; dy++){
	dx = 0;
//	xbuffer = source + dy * x;
//	printStuff = (dy == 256 || dy == 700);
	vector<voxel> rpeaks = findPeaks(source + dy * x, x, pr, minPeakValue, maxEdgeValue, printStuff);
	// and then append the rowPeaks accordingly..
	for(uint i=0; i < rpeaks.size(); i++){
	    rowPeaks.push_back(rpeaks[i]);
//	    rowPeaks.push_back(dy * x + rpeaks[i]);
	    rowPeaks.back().pos += (dy * x);
	}
    }

// 	while(dx < x-1){
// 	    int br = 0;        // radius before.. 
// 	    int xo = dy * x + dx;
// 	    while(dx < x-1 && source[xo + 1] > source[xo]){
// //		cout << "  " << source[xo];
// 		br++;
// 		dx++;
// 		xo++;
// 	    }
// //	    cout << "  " << source[xo] << "  --> " << source[xo + 1];
// 	    // so at this point we've got to a local maxima (at position dx) or we've got to the end of the row..
// 	    // if at end of row, then continue.. or if the width of the increase is too small.. (i.e. br < pr).
// 	    if(dx >= x-1 || br < pr){
// 		dx++;
// //		cout << "\tdx : " << dx << "  br : " << br << endl;
// 		continue;
// 	    }
// 	    // if maxima is too small or edge value (dx - pr) is too large then continue..
// 	    if(source[xo] < minPeakValue || source[xo-pr] > maxEdgeValue){
// 		dx++;
// //		cout << "\tdx : " << dx << "  value : " << source[xo] << "  edge value : " << source[xo-pr] << endl;
// 		continue;
// 	    }
// 	    // and now let's see for how long it goes down..
// 	    int ar=0;         // after radius.. 
// 	    int pp=dx;        // where pp is the peak position..
// 	    while(dx < x-1 &&  source[xo + 1] < source[xo]){
// //		cout << "  " << source[xo];
// 		ar++;
// 		dx++;
// 		xo++;
// 	    }
// //	    cout << " --| " << source[xo+1];
// 	    // if we get here then check things..
// 	    if(ar < pr){
// //		cout << "\tar < pr " << ar << " < " << pr << endl;
// 		dx++;
// 		continue;
// 	    }
// 	    if(source[dy * x + pp + pr] > maxEdgeValue){
// //		cout << "\tmax edge value too high " << source[dy * x + pp + pr] << " > " << maxEdgeValue << endl;
// 		dx++;
// 		continue;
// 	    }
// 	    // if I'm here then we have a peak at position pp, that satisfies all the criteria..
// //	    cout << "\t peak detected " << endl;
// 	    rowPeaks.push_back(dy * x + pp);  // which is the proper peak notification..
// 	}
//     }
    // then do the same but in the opposite direction. This will be a little bit slower since things have to be done 
    vector<voxel> colPeaks;
    for(dx=0; dx < x; dx++){
	for(dy=0; dy < y; dy++){
	    ybuffer[dy] = source[dy * x + dx];
	}
	vector<voxel> cpeaks = findPeaks(ybuffer, y, pr, minPeakValue, maxEdgeValue);
	for(uint i=0; i < cpeaks.size(); i++){
	    colPeaks.push_back(cpeaks[i]);
//	    colPeaks.push_back(x * cpeaks[i] + dx);
	    colPeaks.back().pos = colPeaks.back().pos * x + dx;
	}
    }
// 	dy = 0;
// 	while(dy < y-1){
// 	    int br=0;
// 	    while(dy < y-1 && source[(1 + dy) * x + dx] > source[dy * x + dx]){
// 		br++;
// 		dy++;
// 	    }
// 	    if(dy >= y-1 || br < pr){
// 		dy++;
// 		continue;
// 	    }
// 	    if(source[dy * x + dx] < minPeakValue || source[(dy - pr) * x + dx] > maxEdgeValue){
// 		dy++;
// 		continue;
// 	    }
// 	    int ar = 0;
// 	    int pp = dy;
// 	    while(dy < y-1 && source[(1 + dy) * x + dx] < source[dy * x + dx]){
// 		ar++;
// 		dy++;
// 	    }
// 	    if(ar < pr){
// 		dy++;
// 		continue;
// 	    }
// 	    if(source[(pp + pr) * x + dx] > maxEdgeValue){
// 		dy++;
// 		continue;
// 	    }
// 	    colPeaks.push_back(pp * x + dx);
// 	}
//     }
    cout << "imageData::findLocalMaxima .." << wl << " : " << wavelengths[wl] << "\t" << pr << "\t" << minPeakValue << "\t" << maxEdgeValue << endl;
    cout << "size of rowPeaks " << rowPeaks.size() << "\tsize of colPeaks " << colPeaks.size() << endl;
//     // let's just check how many of these are identical..
//     int px, py;
//     px = py = 0;
//     sort(rowPeaks.begin(), rowPeaks.end());
//     sort(colPeaks.begin(), colPeaks.end());
//     vector<int> mergePeaks;
//     while(px < rowPeaks.size() && py < colPeaks.size()){
// 	int rx = rowPeaks[px] % x;
// 	int ry = rowPeaks[px] / x;
// 	int cx = colPeaks[py] % x;
// 	int cy = colPeaks[py] / x;
// //	cout << px << " : " << py << "\t" << rx << "," << ry << "\t" << cx << "," << cy << "\t" << rowPeaks[px] << "\t" << colPeaks[py] << endl;
// 	if(rowPeaks[px] == colPeaks[py]){
// 	    mergePeaks.push_back(rowPeaks[px]);
// 	    ++px;
// 	    ++py;
// //	    cout << "----" << endl;
// 	    continue;
// 	}
// 	if(rowPeaks[px] < colPeaks[py]){
// 	    ++px;
// 	    continue;
// 	}
// 	++py;
//     }
//     cout << "size of merged Peaks : " << mergePeaks.size() << endl;
    peaks = linearPeaks(x, y, rowPeaks, colPeaks);
//    peaks = linearPeaks(x, y, rowPeaks, colPeaks, mergePeaks);

//    delete xbuffer;
    delete ybuffer;
    return(peaks);
}

linearPeaks imageData::findLocalMaxima(float* source, int pr, float minPeakValue, float maxEdgeValue){
    // this is an optimised peak searching algorithm.
    //
    // 1. Look for linear peaks in rows and remember those
    // 2. Look for linear peaks in columns and remember those
    // 3. Pixels which are peaks in both rows and columns are considered two dimensional peaks.
    // 
    
    // For now we can only handle raw data. That is to say that we are not going to bother trying to deal with
    // merged channels. The reason for this is because of the way in which data is formatted (which makes for ugly
    // semantics). However, we should be able to change this in the future.. 

    linearPeaks peaks;
    float* ybuffer = new float[y];   // and y values here..

    // First find in rows (this is easy...)...
    int dy;   
    int dx;
    vector<voxel> rowPeaks;
    for(dy=0; dy < y; dy++){
	dx = 0;
	vector<voxel> rpeaks = findPeaks(source + dy * x, x, pr, minPeakValue, maxEdgeValue);
	// and then append the rowPeaks accordingly..
	for(uint i=0; i < rpeaks.size(); i++){
	    rowPeaks.push_back(rpeaks[i]);
//	    rowPeaks.push_back(dy * x + rpeaks[i]);
	    rowPeaks.back().pos += (dy * x);
	}
    }

    // then do the same but in the other direction. This will be a little bit slower since things have to be done 
    vector<voxel> colPeaks;
    for(dx=0; dx < x; dx++){
	for(dy=0; dy < y; dy++){
	    ybuffer[dy] = source[dy * x + dx];
	}
	vector<voxel> cpeaks = findPeaks(ybuffer, y, pr, minPeakValue, maxEdgeValue);
	for(uint i=0; i < cpeaks.size(); i++){
	    colPeaks.push_back(cpeaks[i]);
//	    colPeaks.push_back(x * cpeaks[i] + dx);
	    colPeaks.back().pos = (cpeaks[i].pos * x + dx);
	}
    }
    //cout << "imageData::findLocalMaxima .." << wl << " : " << wavelengths[wl] << "\t" << pr << "\t" << minPeakValue << "\t" << maxEdgeValue << endl;
    //cout << "size of rowPeaks " << rowPeaks.size() << "\tsize of colPeaks " << colPeaks.size() << endl;
    // let's just check how many of these are identical..
    peaks = linearPeaks(x, y, rowPeaks, colPeaks);

    delete ybuffer;
    return(peaks);
}

vector<int> imageData::findPeaks(vector<voxel>& line){
    vector<int> peaks;
    if(!line.size()){
	return(peaks);
    }
    if(line.size() == 1){
	peaks.push_back(0);  // the index..
	return(peaks);
    }
    int i = 0;
    while(i < (int)line.size() -1){
	while(i < (int)line.size() - 1 && line[i+1].value >= line[i].value){
	    i++;
	}
	if(!peaks.size()){
	    peaks.push_back(i);
	}else{
	    if(i - peaks.back() > 2){    // i.e.. make sure that we have at least two steps between..
		peaks.push_back(i);
	    }
	}
	// since it is formally possible that we may detect another peak..
	while(i < (int)line.size() -1 && line[i + 1].value <= line[i].value){
	    i++;
	}
    }
    return(peaks);
}

void imageData::findAllMaxima(int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm){
    // go through all of the frames and make a linearPeaks structure from each one...
    if(wl >= w || wl < 0){
	cerr << "imageData::findAllMaxima unsuitable wavelength index.. " << wl << endl;
    }
    if(volumePeaks.count(wl)){
	volumePeaks[wl].peaks.resize(0);
    }
    int dropRadius = 4;   // should be big enough.. 

    volumePeaks[wl].radius = dropRadius;    // it gets created if not present, hence I don't have to call the constructor.. 
                                            // convenient, but it was causing me a bit of a headache for a while. 
    cout << "imageData::findAllMaxima" << endl;
    float* buffer;       // make sure not to delete buffer,, 
//    float* buffer = new float[x*y];
    vector<linearPeaks> peaks;
    map<int, float> globalPeaks;   // define all peaks and their values by their global position 
    for(int dz=0; dz < z; dz++){
	buffer = readFrame(dz, wl);
	// and then find peaks in the buffer.. would be neater to use a separate function for this,, hmm. 
	peaks.push_back(findLocalMaxima(buffer, pr, minPeakValue, maxEdgeProportion));
	vector<voxel>::iterator it;
	for(it = peaks.back().xyPeaks.begin(); it != peaks.back().xyPeaks.end(); it++){
	    globalPeaks.insert(make_pair((*it).pos + x * y * dz, (*it).value));
	}
	cout << "section : " << dz << "\t";
	peaks.back().printStats();
    }
    // at this point we can make an attempt at defining peaks in 3 dimensions.. 
    // make a map of vector of voxels, where the int defines the start of the peak profile
    // then do peak finding on each of these, as it is possible that each one may contain more than one peak..
    map<int, vector<voxel> > peakLines;
    map<int, float>::iterator it;
    set<int> inserted;   // peaks which have already been inserted into the thingy..
    int fs = x * y;  // frame size.. 
    for(it = globalPeaks.begin(); it != globalPeaks.end(); it++){
	int pkstart = (*it).first;
	if(inserted.count(pkstart)){
	    continue;
	}
	peakLines[pkstart].push_back(voxel(pkstart, (*it).second));
	inserted.insert(pkstart);
	int pkpos = pkstart + fs;
	// this algorithm is a little bit too simplistic as the peak lines don't always go straight through the 
	// sample but can be shifted by one pixel to any of the intervening positions..
	bool checkMore = true;
	while(checkMore){       // check all 9 possible bording pixels..  NOTE that we don't bother checking all positions in the original plane,, shouldn't be necessary
	    checkMore = false;  // the default.. 
	    for(int i=-1; i < 2; i++){
		for(int j=-1; j < 2; j++){
		    int pos = pkpos + i * x + j;
		    if(globalPeaks.count(pos)){
			peakLines[pkstart].push_back(voxel(pos, globalPeaks[pos]));
			inserted.insert(pos);
			checkMore = true;
		    }
		}
	    }
	    pkpos += fs;
	}
// 	while(globalPeaks.count(pkpos)){
// 	    peakLines[pkstart].push_back(voxel(pkpos, globalPeaks[pkpos]));
// 	    inserted.insert(pkpos);
// 	    pkpos += fs;
// 	}
    }
    // at this point we have a number of lines in the z-dimension representing peaks in the x-y plan. I now need to go through these lines and 
    // find the number of peaks within them. I think that I can probably use quite a simple peak finding algorithm, similar to the one above.
    // hmm, I can actually convert each voxel to a line (float* in a loop)..  -- first let's just look at the individual peak lines..
    cout << "Total of " << peakLines.size() << "  peak lines" << endl;
    map<int, vector<voxel> >::iterator mit;
    int totalPeakNo = 0;
    for(mit = peakLines.begin(); mit != peakLines.end(); mit++){
	int pkStart = (*mit).first;
	int frameStart = pkStart / fs;
	int xypos = pkStart % fs;
	int xpos = xypos % x;
	int ypos = xypos / y;
	vector<voxel>::iterator vit;
	vector<int> peaks = findPeaks((*mit).second);
	if(peaks.size() > 1){
	    cout << "peaks size is larger than 1 :\t";
	    cout << "Peak line frame : " << frameStart << " x: " << xpos << "  y: " << ypos;
	    for(vit = (*mit).second.begin(); vit != (*mit).second.end(); vit++){
		cout << "\t" << (*vit).value;
	    }
	    cout << endl;
	}
	for(uint i=0; i < peaks.size(); i++){
	    // add the peak to the 3Dpeak structure...........
	    volumePeaks[wl].peaks.push_back((*mit).second[peaks[i]].pos);
	    totalPeakNo++;
	}
    }
    cout << "Total peak Number is : " << totalPeakNo << endl;
    setDropVolumes(bgm);

    // let's try to do a k-means cluster on these... 
    //int K = 10;
    // at which point we could do something useful, but let's wait for it..
//    for(int i=0; i < cluster->k; i++){
//	cout << "Cluster " << i << "\tsize : " << cluster->clusterSizes[i] << "\tmaxD : " << cluster->maxDistances[i] << endl;
//    }
    
}

void imageData::findAllPeaks(int wl, int pr, float minPeakValue, float maxEdgeProportion, float bgm){
    if(wl >= w || wl < 0){
	cerr << "imageData::findAllPeaks unsuitable wavelength index.. " << wl << endl;
    }
    bgm = bgm;  /// to avoid warning of unused veriable..

    // find peaks in all three dimensions. Define overlapping positions.
    
    // Then use the peak information to grow the actual peaks and to define these.
    //
    // basically grow the peak, if appropriate neighbouring peaks are available.
    
    // 1. First define the three different types of peaks..
    map<long, linearPeak> xPeaks;
    map<long, linearPeak> yPeaks;
    map<long, linearPeak> zPeaks;  // this could probably be done faste and neater with sorted vectors, but I've started so..
    
    float* xline = new float[x];
    float* yline = new float[y];
    float* zline = new float[z];

//     vector<long> xlong;
//     vector<long> ylong;
//     vector<long> zlong;

    cout << "findAllPeaks pr is " << pr  << endl;
    // First the X and Y peaks..
    for(int dz=0; dz < z; dz++){
	// X peaks..
//	cout << "findAllPeaks getting xLines" << endl;
	for(int dy=0; dy < y; dy++){
	    // here find xpeaks..
	    if(simpleLine(xline, 0, dy, dz, wl)){
		vector<linearPeak> peaks = findPeaks(xline, x, pr, minPeakValue, maxEdgeProportion, 0);
		// this is a bit ugly, but we need to change the positions of all the values in the peak..
		long offset = dz * x * y + dy * x;
		for(uint i=0; i < peaks.size(); i++){
		    if(peaks[i].position - peaks[i].begin > pr){
			cout << "peak positions " << peaks[i].begin << " -- " << peaks[i].position << "  -- " << peaks[i].end << endl;
			exit(1);
		    }
		    
		    peaks[i].position += offset;
		    peaks[i].begin += offset;
		    peaks[i].end += offset;
		    xPeaks.insert(make_pair(peaks[i].position, peaks[i]));
//		    xlong.push_back(peaks[i].position);
		}
	    }
	}
	for(int dx=0; dx < x; dx++){
	    // here find ypeaks..
	    //    cout << "findAllPeaks getting yLines" << endl;
	    if(simpleLine(yline, 1, dx, dz, wl)){
		vector<linearPeak> peaks = findPeaks(yline, y, pr, minPeakValue, maxEdgeProportion, 1);
		for(uint i=0; i < peaks.size(); i++){
		    if(peaks[i].position - peaks[i].begin > pr){
			exit(1);
		    }
		    
		    long sliceOffset = dz * x * y;
		    peaks[i].position =  sliceOffset + x * peaks[i].position + dx;
		    peaks[i].begin = sliceOffset + x * peaks[i].begin + dx;
		    peaks[i].end = sliceOffset + x * peaks[i].end + dx;
		    yPeaks.insert(make_pair(peaks[i].position, peaks[i]));
//		    ylong.push_back(peaks[i].position);
		}
	    }
	}
    }
    // And then the Z -peaks..
//    cout << "and going for lots of z lines " << endl;
    for(int dy=0; dy < y; dy++){
	for(int dx=0; dx < x; dx++){
	    // here find ypeaks..
	    if(simpleLine(zline, 2, dx, dy, wl)){
		vector<linearPeak> peaks = findPeaks(zline, z, pr, minPeakValue, maxEdgeProportion, 2);
		for(uint i=0; i < peaks.size(); i++){
		    if(peaks[i].position - peaks[i].begin > pr){
			exit(1);
		    }
		    
		    peaks[i].position = x * y * peaks[i].position + dy * x + dx;
		    peaks[i].begin = x * y * peaks[i].begin + dy * x + dx;
		    peaks[i].end = x * y * peaks[i].end + dy * x + dx;
		    zPeaks.insert(make_pair(peaks[i].position, peaks[i]));
//		    zlong.push_back(peaks[i].position);
		}
	    }
	}
    }
    cout << endl << endl << "zPeaks size is : " << zPeaks.size() << endl << endl;

//    cout << "calling delete on the lines .. " << endl;

    delete xline;
    delete yline;
    delete zline;

//     int xp, yp, zp;
//     xp = yp = zp = 0;
//     sort(xlong.begin(), xlong.end());
//     sort(ylong.begin(), ylong.end());
//     sort(zlong.begin(), zlong.end());
//     int comCounter = 0;
//     while(xp < xlong.size() && yp < ylong.size() && zp < zlong.size()){
// 	if(xlong[xp] == ylong[yp] && xlong[xp] == zlong[zp]){
// 	    comCounter++;
// 	    xp++; yp++; zp++;
// 	    continue;
// 	}
// 	if(xlong[xp] <= ylong[yp] && xlong[xp] <= zlong[zp]){
// 	    xp++;
// 	}
// 	if(ylong[yp] <= xlong[xp] && ylong[yp] <= zlong[zp]){
// 	    yp++;
// 	}
// 	if(zlong[zp] <= xlong[zp] && zlong[zp] <= ylong[yp]){
// 	    zp++;
// 	}
//     }

    /// this doesn't work, as if two things are equal to each other, then once one is incremented, the other
    /// is not equal to it.. it screws the last position if x and y become equal to z, then z will still be incremented. 

//    cout << "lines deleted and looking for common peaks.. " << endl;
    // at this point we should define a set of points of overlapping peaks... 
    set<long> commonPeaks;



    map<long, linearPeak>::iterator xIt = xPeaks.begin();
    map<long, linearPeak>::iterator yIt = yPeaks.begin();
    map<long, linearPeak>::iterator zIt = zPeaks.begin();
    // and then simply..
    while(xIt != xPeaks.end() && yIt != yPeaks.end() && zIt != zPeaks.end()){
	// if all are equal to each other, then great..
	if((*xIt).first == (*yIt).first && (*xIt).first == (*zIt).first){
	    commonPeaks.insert((*xIt).first);
	    xIt++; yIt++; zIt++;
	    continue;
	}
	// at this point we want to increase the iterator with the lowest value..
	if((*xIt).first <= (*yIt).first && (*xIt).first <= (*zIt).first){
	    xIt++;
	    continue;
	}
	if((*yIt).first <= (*xIt).first && (*yIt).first <= (*zIt).first){
	    yIt++;
	    continue;
	}
	if((*zIt).first <= (*xIt).first && (*zIt).first <= (*yIt).first){
	    zIt++;
	}
    }
    cout << "found a load of commone peaks size : " << commonPeaks.size() << endl;
    // at which point we should have a set of common peaks that we can now go through and grow using our already defined linear peaks.
    // so at this point we need some sort of a structure to store the information that we'll be gathering.. 
    
    // in one sense however, we can just use the widths of the three peaks to define a cube containing the droplet, 
    // we could then just take the sum of all the values within the cube, or do some kind of background subtraction or something
    // more complicated to get indicate the signal strength.

    // However, we can still end up with peaks which are next to each other,, esp. if there is something resembling a diagonal line.
    // one way to prevent this is to look within this peak cube area and make sure if there are any other 3D peaks within it.

    // However, it is not clear what should be done if this is the case. One alternative is to keep the highest peak, another alternative is
    // to keep neither peak, as this could indicate something with the wrong shape, or something like a diagonal line of some sort.
    //
    // a third alternative is to consider it as a single peak, and increase the peak dimensions to reflect this. 
    
    // keep the bigger/more intense one... (determine after calculation of the given thing)
    //
    // Note that this still allows some overlap between peaks, as we are only checking for the presence of another peak in the thingy.. 
    
    // let's make a map of simple_drops
    map<long, simple_drop> drops;
    float* source = data + wl * x * y * z;
    for(set<long>::iterator it=commonPeaks.begin(); it != commonPeaks.end(); it++){
	// this means that we can look up the components from xPeaks, yPeaks and zPeaks
	long pos = *it;   // easier to write
	int zp = pos / ( x * y );
	int yp = (pos % (x * y)) / x;
	int xp = (pos % (x * y)) % x;
	// the begin values
	int xb = (xPeaks[pos].begin % (x * y)) % x;
	int xe = (xPeaks[pos].end % (x * y)) % x;
	int yb = (yPeaks[pos].begin % (x * y)) / x;
	int ye = (yPeaks[pos].end % (x * y)) / x;
	int zb = zPeaks[pos].begin / (x * y);
	int ze = zPeaks[pos].end / (x * y);
	
//	cout << "drop at position : " << pos << "  " << xb << "-" << xp << "-" << xe << "  :  " << yb << "-" << yp << "-" << ye << "  :  " << zb << "-" << zp << "-" << ze << endl;


	// now we have a somewhat tricky business. I want to make vector of values. 
	int diameter = 2 * pr + 1;
	int area = diameter * diameter;
	int volume = diameter * diameter * diameter;
	vector<float> values(volume, 0);
	for(int dz=(zb - zp); dz <= (ze - zp); dz++){
	    for(int dy=(yb - yp); dy <= (ye - yp); dy++){
		for(int dx=(xb - xp); dx <= (xe - xp); dx++){
//		    cout << "assigning from : " << (dz + zp) * x * y + (dy + yp) * x + dx + xp << "  --> " << (dz + pr) * area + (dy + pr) * diameter + pr + dx << endl;
		    values[ (dz + pr) * area + (dy + pr) * diameter + pr + dx ] = source[(dz + zp) * x * y + (dy + yp) * x + dx + xp];  // which shouldn't go out of bounds..
		}
	    }
	}
	// and at which point we should assign a new simple_drop..
	drops.insert(make_pair(*it, simple_drop(pr, xp, yp, zp, xb, xe, yb, ye, zb, ze, values)));
    }
    // and  at this point we might want to clean up drops and make check whether or not we want to play with this. 
    // Note that everything in the loop above can be moved into the loop looking for common peaks, as I don't do anything
    // particularly strange here. But for now it seems simpler to keep the two loops separately, and in the grand scheme of things
    // this isn't likely to affect the speed of the process too much.
    cout << endl << endl << "find all peaks found a total of " << drops.size() << "  peaks " << endl << endl;
    volumePeaks[wl].simpleDrops = drops; 
    volumePeaks[wl].xPeaks = xPeaks;
    volumePeaks[wl].yPeaks = yPeaks;
    volumePeaks[wl].zPeaks = zPeaks;
}



KClusterProcess* imageData::clusterDrops(int wi, int K){
    if(!volumePeaks.count(wi)){
	cerr << "No volume peaks with wave index : " << wi << " can't do any clustering bugger " << endl;
    }
    cout << "making a cluster process for the drops.. " << endl;
    KClusterProcess* cluster = new KClusterProcess(K, volumePeaks[wi].drops);
    cout << "just before calling start on the cluster process " << endl;
    cluster->start();
    cluster->wait();
    //  obviously we don't have to call wait and do that here, we could just return the pointer 
    //  and let whoever is downstream handle it as they like.. but for the moment, I will just do everything here
    // instead.. 
    cout << "and the cluster process has returned ok.." << endl;
    return(cluster);
}

void imageData::recalculateSpotVolumes(){
    // 
    cout << "imageData:: recalculateSpotVolumes .. " << endl;
    // awful bloody hack. This assumes that we have some volumePeaks..
    // this is bad, since we are not making much sense..
    setDropVolumes(4.0);   // the 4 is ugly as hell.. 
}

void imageData::setDropVolumes(float bgm){
    cout << "beginning of set dropVolumes" << endl;
//    float* buffer;
    float* buffer = new float[x * y * z];   // which will be pretty bloody big, but at least it's only for one wavelength..
    map<int, threeDPeaks>::iterator it;
    for(it = volumePeaks.begin(); it != volumePeaks.end(); it++){
	int radius = (*it).second.radius;        // which is easier to refer to .. 
	// first read in the data ..
//	cout << "checking peaks for waveIndex : " << (*it).first << endl;
	if((*it).first >= 0 && (*it).first < w){
	    for(int fno=0; fno < z; fno++){
		memcpy(buffer + fno * x * y, data + (*it).first * x * y * z + fno * x * y, sizeof(float) * x * y);
//		cout << "reading frame : " << fno << endl;
//		readFrame(fno, (*it).first, buffer + fno * x * y);
	    }
	    // remove all old drops..
	    (*it).second.drops.resize(0);
	    (*it).second.drops.reserve((*it).second.peaks.size());

	    float sf = scaleFactors[(*it).first];
	    float bf = biasFactors[(*it).first];
	    
	    // and now we can go through all the thingies..
	    vector<int>::iterator vit;
	    float xVar, yVar, zVar;
	    xVar = yVar = zVar = 0;
	    float dno = float((*it).second.peaks.size());
	    for(vit = (*it).second.peaks.begin(); vit != (*it).second.peaks.end(); vit++){
		int dz = (*vit) / (x * y);        // the frame number..
		int dy = ((*vit) % (x * y)) / x;  // the row number
		int dx = ((*vit) % (x * y)) % x;  // the column number (x position).. 
		// First we need to push back the appropriate peaks.. 
		(*it).second.drops.push_back(dropVolume((*vit), radius, dx, dy, dz, bgm));  // which should be ok.. 
//		cout << "drop ";
		//cout << "Assigning drop values : " << endl;
//		int rowCount = 0;
		for(int ddz=-radius; ddz <= radius; ddz++){
		    int fn = dz + ddz;
//		    cout << "  z: " << fn << " " << dz << " " << ddz;
		    if(fn < 0 || fn >= z){
//			cout << "fn is larger than w " << fn << " > " << w << endl;
			continue;
		    }
		    for(int ddy=-radius; ddy <= radius; ddy++){
			int rn = dy + ddy;
//			cout << " y : " << rn << " " << dy << " " << ddy;
			if(rn < 0 || rn >= y){
			    continue;
			}
//			cout << rowCount++;
			for(int ddx=-radius; ddx <= radius; ddx++){
			    int cn = dx + ddx;   // running out of variable names.. (these numbers should be the coordinates in the buffer).
//			    cout << " x: " << cn << " " << dx << " " << ddx;
			    if(cn < 0 || cn >= x){
				continue;
			    }
			    // so at this point we have to work out the different offsets.. (a local and a global one)
			    int globalOffset = fn * x * y + rn * x + cn;
			    int diameter = 2 * radius + 1;
			    int localOffset = (ddz + radius) * (diameter * diameter) + (ddy + radius) * diameter + (ddx + radius);
			    // and then simply ..
			    float addTerm = buffer[globalOffset] * sf + bf;   // should be ok.. 
			    (*it).second.drops.back().values[localOffset] = buffer[globalOffset];
//			    cout << "\t" << localOffset << ":"  << buffer[globalOffset];
			    if(addTerm > 0){
				(*it).second.drops.back().totalValue += addTerm;
			    }
//			    (*it).second.drops.back().totalValue += buffer[globalOffset];
//			    cout << "\t" << buffer[globalOffset] << ":" << (*it).second.drops.back().totalValue;
			}
//			cout << endl;

		    }
		}
		// and at this point we can call setShapes on the drop.. and report that..
		//cout << "calling setShapes " << endl;
		(*it).second.drops.back().setShapes();
		xVar += (*it).second.drops.back().xVariance / dno;
		yVar += (*it).second.drops.back().yVariance / dno;
		zVar += (*it).second.drops.back().zVariance / dno;
		// let's override that with a function that uses a gaussian approximation..
		// but I need to calculate the appropriate variances and stuff, so not very appropriate to do here.. 
		// anyway, I should move a load of stuff away from here.. 
		// and let's go through..
		//	(*it).second.drops.back().printShapes();
//		(*it).second.drops.back().totalValue = log((*it).second.drops.back().totalValue);
//		cout << endl;
	    }
	    vector<dropVolume>::iterator dit;
	    // this overrides the total Sum thingy used as the value of the thingy.. 
	    for(dit = (*it).second.drops.begin(); dit != (*it).second.drops.end(); dit++){
		(*dit).evaluateGaussianModel(xVar, yVar, zVar, 0.1);
	    }
//	    cout << endl;
	}
    }
    // which would seem to be everything I need to do..
    delete buffer;
}

vector<float> imageData::spotValues(int wl){
    vector<float> v;
    map<int, threeDPeaks>::iterator it = volumePeaks.find(wl);
    if(it == volumePeaks.end()){
	cerr << "imageData::spotValues no spots for wave index : " << wl << endl;
	return(v);
    }

     v.reserve((*it).second.simpleDrops.size());
    map<long, simple_drop>::iterator vit;
    for(vit = (*it).second.simpleDrops.begin(); vit != (*it).second.simpleDrops.end(); vit++){
	v.push_back((*vit).second.sumValue);
    }

//     v.reserve((*it).second.drops.size());
//     vector<dropVolume>::iterator vit;
//     for(vit = (*it).second.drops.begin(); vit != (*it).second.drops.end(); vit++){
// 	v.push_back((*vit).totalValue);
//     }
    return(v);
}

voxelVolume imageData::makeModel(int xBegin, int width, int yBegin, int height, int zBegin, int depth, set<int> waveIndices){
    //make a voxelVolume for the given coordinates. In this volume include :
    
    // All peaks given in the map<int, threeDPeaks> volumePeaks
    // All pixels given in the waveIndices set.

    // Since we haven't yet implemented a system for giving specific colours for models use the colours specified for 
    // each wavelength and use given scaling parameters for the defining the final colours.
    // However, rather than scaling the actual colours, only scale the alpha parameter.
    
    // To begin with use some specified number of pixels around the core of each model rather than doing something fancy stuff
    // to decide where the edge of the dot should lie. Scale using normal parameters for scaling.. 


    // For the waveIndices indicated include pixels which give at least some positive value after thingy (use the thingy to determine this).
    
    // In all cases only make voxels that have some value,, ok La ?
    voxelVolume nullVolume;
    if(xBegin < 0 || yBegin < 0 || zBegin < 0 || height < 0 || depth < 0 || width < 0){
	cerr << "imageData::makeModel -negative parameter received " << endl;
	return(nullVolume);
    }
    if(zBegin + depth > z){
	cerr << "setting depth to something else.. " << endl;
	zBegin = 0;
	depth = z;
    }
    // don't bother to check if it fits.. we don't care, just go ahead and do the wors that can be done..
    
    voxelVolume vol(width, height, depth, float(width) * x_scale, float(height) * y_scale, float(depth) * z_scale);    

    // in fact first we have to read in the data into a temporary buffer or we are going to go mad trying to 
    // do millions of reads from disk..
    float* buffer = new float[width * height * depth];
    // unfortunately we probably have to read for specific channels, so .. for this reason we better get back to being boring bastards..
    

    // that anyway is the way that the model should be considered..
    map<int, threeDPeaks>::iterator it;    // the int refers to the waveIndex..
    for(it = volumePeaks.begin(); it != volumePeaks.end(); it++){
	// check thhat it is smaller than w..
	if((*it).first >= 0 && (*it).first < w){
	    float r = colors[3 * (*it).first];
	    float g = colors[3 * (*it).first + 1];
	    float b = colors[3 * (*it).first + 2];
	    float sf = scaleFactors[(*it).first];
	    float bf = biasFactors[(*it).first];
	    float minThreshold = (*it).second.minThreshold;
	    cout << "making model waveIndex : " << (*it).first << "  r : " << r << "  g: " << g << "  b : " << b << endl;
	    // and then .. fill the buffer by reading through the frmaes..
	    for(int fno=zBegin; fno < zBegin + depth; fno++){
		readFrame(fno, (*it).first, xBegin, yBegin, width, height, buffer + (fno - zBegin) * width * height);
	    }
	    // so now we should be able to do everything that we could possibly want to do with this data set ..
	    vector<dropVolume>::iterator vit;
	    for(vit = (*it).second.drops.begin(); vit != (*it).second.drops.end(); vit++){
		if((*vit).totalValue < minThreshold){    // bit dangerous... hmm.. 
		    continue;
		}
		int dz = (*vit).center / (x * y);        // the frame number..
		int dy = ((*vit).center % (x * y)) / x;  // the row number
		int dx = ((*vit).center % (x * y)) % x;  // the column number (x position).. 
		// check if the core is within the volume given..
		if(dx >= xBegin && dx <= xBegin + width && dy >= yBegin && dy <= yBegin + height && dz >= zBegin && dz <= zBegin + depth){
		    // and now include a given number of voxels around the thingy..
		    int radius = 4;   // read 4 pixels in every direction (9 * 9 * 9 total size - probably more than necessary but.. )
		    for(int ddz=-radius; ddz <= radius; ddz++){
			int fn = dz + ddz - zBegin;
			if(fn < 0 || fn >= depth){
			    continue;
			}
			for(int ddy=-radius; ddy <= radius; ddy++){
			    int rn = dy + ddy - yBegin;
			    if(rn < 0 || rn >= height){
				continue;
			    }
			    for(int ddx=-radius; ddx <= radius; ddx++){
				int cn = dx + ddx - xBegin;   // running out of variable names.. (these numbers should be the coordinates in the buffer).
				if(cn < 0 || cn >= width){
				    continue;
				}
				// and then add a voxel to the model... arghh.... 
				// ohalalala too many lines of code man..
				//if(fn >=0 && fn < depth && rn >= 0 && rn < height && cn >= 0 && cn < width){   // oooh so ugly...
				// and now we actually have to calculate the intensity of thhe dot we want to make..
				float v = buffer[fn * width * height + rn * width + cn];
				float alpha = v * sf + bf;
				if(alpha > 0){
				    vol.voxels.push_back(drawVoxel(cn, rn, fn, r, g, b, alpha));
				}
				//	}
			    }
			}
		    }
		}
	    }
	}
    }
    // and at this point we should do the same but for all elements of the buffer, for wavelengths given in the set<int> waveIndices .. 
    
    // but the code is so ugly...
    set<int>::iterator sit;
    for(sit = waveIndices.begin(); sit != waveIndices.end(); sit++){
	if((*sit) > 0 && (*sit) < w){
	    float r = colors[3 * (*sit)];
	    float g = colors[3 * (*sit) + 1];
	    float b = colors[3 * (*sit) + 2];
	    float sf = scaleFactors[(*sit)];
	    float bf = biasFactors[(*sit)];
	    for(int fno=zBegin; fno < zBegin + depth; fno++){
		readFrame(fno, (*sit), xBegin, yBegin, width, height, buffer + (fno - zBegin) * width * height);
	    }
	    // and then simply go through the buffer and check what goes where..
	    for(int dz=0; dz < depth; dz++){
		for(int dy=0; dy < height; dy++){
		    for(int dx=0; dx < width; dx++){
			// and then calculate .. 
			float v = buffer[dz * width * height + dy * width + dx];
			float alpha = v * sf + bf;
			if(alpha > 0){
			    vol.voxels.push_back(drawVoxel(dx, dy, dz, r, g, b, alpha));
			}
		    }
		}
	    }
	}
    }
    delete buffer;
    cout << "makeModel buffer deleted size of voxels :  " << vol.voxels.size() << endl;
    return(vol);
}

void imageData::setDropThresholds(int wi, float minT, float maxT){
    cout << "imageData setting thresholds for drops : " << minT << "\t" << maxT << endl;
    map<int, threeDPeaks>::iterator it = volumePeaks.find(wi);
    if(it == volumePeaks.end()){
	cerr << "imageData::setDropThresholds no model for wave index : " << wi << endl;
	return;
    }
    // note that maxT must be equal to or larger than minT..
    if(minT > maxT){
	cerr << "imageData::setDropThresholds min threshold : " << minT << "  >  " << maxT << "  max threshold " << endl;
	return;
    }
    (*it).second.minThreshold = minT;
    (*it).second.maxThreshold = maxT;
    // and then whomever can do whatever..
}


void imageData::findNuclearPerimeters(int wi, float minValue){
    // first check that wi is ok..
    if(wi < 0 || wi >= w){
	cerr << "imageData::findNuclearPerimeters inappropriate wave index obtained : " << wi << endl;
	return;
    }
    // use the data in frame buffer.. so make a local pointer..
    float* source = frameBuffer + wi * x * y;  // which should be fine..
    
    // we are going to be looking at data only within the globalMargin ignoring data outside of it.
    //
    // perimeters are found using a simple linear scan.. though I'm not sure exactly how to do this yet..
    //
    // First just find a load of horizontal lines and then merge these together to form areas, and from these make perimeters...
    // Sounds pretty simple, but bound to run into difficulties along the lines.. 
    
    // go throuh one line at a time..
    vector<line> lines;  // which we are going to play with now.. 
    int minLength = 5;
    for(int row = globalMargin; row < y-globalMargin; row++){
	bool inLine = false;   // i.e. we are not currently in a line..
	int start = 0;
	int stop = 0;
	for(int col=globalMargin; col < x-globalMargin; col++){
	    if(!inLine){
		if(source[row * x + col] >= minValue){  // start a line and define start..
		    inLine = true;
		    start = row * x + col;
		}
		continue;
	    }
	    // so here must be in the middle of a line.. -- simplest thing to do is 
	    // to just say.. hmm if lower than threshold than stop,, but we should perhaps look forward a little bit.. 
	    // so how about take the average of the next 5 or so values... and if that is more than limit than extend by one
	    // screw that for now, let's just include it if it is somewhere along the other stuff..
	    if(source[row * x + col] < minValue){
		// i.e. we have to terminate the line..
		stop = row * x + col;
		if(stop - start >= minLength){
		    lines.push_back(line(start, stop, x));
		}
		start = stop = 0;
		inLine = false;
	    }
	}
	// here we have to check if we still have a line continuing
	if(inLine){
	    stop = row * x +  x-globalMargin-1;
	    if(stop - start >= minLength){
		lines.push_back(line(start, stop, x));
	    }
	}
    }
    nuclearLines = lines;
    cout << "defined a total of " << lines.size() << "  lines " << endl;
    // and let's get the perimenters..
    findNuclearPerimeters();
    for(uint i=0; i < nuclei.size(); i++){
	nuclei[i].inversionScores(source, x, y);
    }

    // this then gives us a set of nuclei defined.. 
    // but we haven't yet gone through and integrated the signal in each one of these..
    // which we can do here..
//    float* source;
//    source = new float[x * y];
    for(int i=0; i < z; i++){
	cout << "incrementing signal from frame : " << i << endl;
	source = readFrame(i, wi);
	// and then go through the nuclei..
	for(uint j=0; j < nuclei.size(); j++){
	    nuclei[j].incrementSignal(source, x, y);
	}
    }
    for(uint i=0; i < nuclei.size(); i++){
	cout << "Nucleus # " << i << "  signal :\t" << nuclei[i].totalSignal << endl;
	// and let's see if we can do the thingy..
    }
//    delete source;
}

void imageData::findNuclearPerimeters(){
    // look at the nuclear lines and make perimeters from these that can be drawn.. (might want to 
    // integrate the signal strength over the nuclei as well.. 

    cout << "beginning of findNuclearPerimeters" << endl;
    // in general we shouldn't need to sort the lines, but to begin with we'll start..
    sort(nuclearLines.begin(), nuclearLines.end());
    cout << "nuclearLines sorted " << endl;
    // first go through lines and merge so that internal holes can be sorted out.. 
    set<line> mergedLines;   // lines which have already been merged..
    set<set<line> > nuclearSets;
    for(uint i=0; i < nuclearLines.size(); i++){
//	cout << i << endl;
	if(mergedLines.count(nuclearLines[i])){
	    continue;
	}
//	cout << "finding neigbouring lines " << endl;
	set<line> currentLines;
	nuclearLines[i].neighboringLines(nuclearLines, currentLines);  // which 
	mergedLines.insert(currentLines.begin(), currentLines.end());
	nuclearSets.insert(currentLines);    // which should do a copy I think..
	cout << "Set contains a total of " << currentLines.size() << " lines " << endl;
	currentLines.erase(currentLines.begin(), currentLines.end());
    }
    cout << "findNuclearPerimeters.. obtained a total of " << nuclearSets.size() << " set of nuclear lines " << endl;
    
    // now we need to go through each set and try to work out a perimeter around that set. 
    // there are probably better ways of doing this, but I'll do something like this..
    
    // 1. Go to first line..
    // 2. Find first neighbour below to the first line, make this the current line..
    // 3. Repeat 2, until no more lines neigbhouring below, add start point to perimeter..
    
    // 4. Find last line above.
    //     check if this line has line below that starts to the right of the current line,
    //     if this is the case then make two points on the perimeter and look for lines below that line
    //     if not then find last line that neighbours above and add the end point of this line.
    // 5. Repeat 4 until no more .. 

    // bugger that, it's too complicated, use a different function that makes a bitmap image of the stuff instead..
    nuclearPerimeters.resize(0);
    nuclei.resize(0);
    for(set<set<line> >::iterator it=nuclearSets.begin(); it != nuclearSets.end(); it++){
	cout << endl << "calling findNuclearPerimeter " << endl;
	Nucleus nucleus = findNuclearPerimeter((*it));
//	vector<twoDPoint> perimeter = findNuclearPerimeter((*it));
	cout << "total of " << nucleus.perimeter.size() << " points in perimeter" << endl;
	nuclearPerimeters.push_back(nucleus.perimeter);
	nuclei.push_back(nucleus);
    }
    
}

//vector<twoDPoint> imageData::findNuclearPerimeter(const set<line>& lines){
Nucleus imageData::findNuclearPerimeter(const set<line>& lines){
    // first find out the dimensions.. i.e. the min and max col and row no's.
    Nucleus nucleus;
    set<line>::iterator it;
    vector<twoDPoint> points;
    int minCol, maxCol;
    int minRow, maxRow;
    it = lines.begin();
    if(it == lines.end()){
	cerr << "no lines in line set " << endl;
	return(nucleus);
	//return(points);
    }
    minCol = (*it).col_start;
    maxCol = (*it).col_stop;
    minRow = maxRow = (*it).rn;
    for(it=lines.begin(); it != lines.end(); it++){
	if(minCol > (*it).col_start){ minCol = (*it).col_start; }
	if(maxCol < (*it).col_stop){ maxCol = (*it).col_stop; }
	if(minRow > (*it).rn){ minRow = (*it).rn; }
	if(maxRow < (*it).rn){ maxRow = (*it).rn; }
    }
    nucleus.min_x = minCol;
    nucleus.max_x = maxCol;
    nucleus.min_y = minRow;
    nucleus.max_y = maxRow;
    nucleus.lines = lines;
    cout << "minCol : " << minCol << "  maxCol : " << maxCol << "  minRow : " << minRow << "  maxRow : " << maxRow << endl;
    int nuc_width = 2 + 1 + maxCol - minCol;
    int nuc_height = 2+ 1 + maxRow - minRow;    // the two plus is to make sure that we have 
    // and make a vector of booleans that fills this space..
    vector<bool> nuc_mask(nuc_width * nuc_height, false);
    // and then fill this boolean with stuff..
    for(it = lines.begin(); it != lines.end(); it++){
	int yp = 1 + (*it).rn - minRow;    // the yposition.. translated into the local coordinates .. (with a margin of 1 position) 
	for(int x_p = (*it).col_start; x_p <= (*it).col_stop; x_p++){
	    int xp = 1 + x_p - minCol;    // local coordinates with a one thingy.. 
	    nuc_mask[yp * nuc_width + xp] = true;
	}
    }
    // and now we have a mask we can go around the mask and fill in stuff..
    int xoffsets[] = {-1, -1, 0, 1, 1, 1, 0, -1};
    int yoffsets[] = {0, -1, -1, -1, 0, 1, 1, 1};     // this gives us a anticlockwise spin around a central position.. 
    int offset_offsets[] = {6, 6, 0, 0, 2, 2, 4, 4};  // how the offset position should move as a result of stuff.. 

    //    7 6 5                                 X 6 5
    //    0 X 4   new boundary pos at 3  -->    0 Y 4   then 0 is the offset of the non thingy position.. (hmm). 
    //    1 2 3                                 1 2 3

    
    // basically we will find a starting point, then from this point we'll follow the offsets around until 
    // we hit the pattern, false followed by true. the true position is our next point in the thingy.. 
    
    // First we have to find a starting position..
    int start_pos = 0;
    for(uint i=0; i < nuc_mask.size(); i++){
	if(nuc_mask[i]){
	    start_pos = i;
	    break;
	}
    }
    points.push_back(twoDPoint(minCol - 1 + (start_pos % nuc_width), minRow - 1 + (start_pos / nuc_width)));
    // and then use the offsets to find the next one..
    int col_pos = start_pos % nuc_width;
    int row_pos = start_pos / nuc_width;
    int offsetPos = 0;     // which is the position on the left of the current position.. 
    // then try to find the next position..
    bool keep_going = true;
    while(keep_going){
	// go through the 8 positions..	
	for(int i=0; i < 8; i++){
	    int op = (offsetPos + i) % 8;    // somewhere between 0 and 7 .. 
	    // because the original position has to be false we can just say if we come across
	    // a true position that is our next member.. 
	    if(nuc_mask[ (row_pos + yoffsets[op]) * nuc_width + (col_pos + xoffsets[op])]){
		// then this position is our new boundary thingy.. 
		// and the op - 1 is our new offsetPos .. 
		// first check if we've gone all the way around..
		if((row_pos + yoffsets[op]) * nuc_width + (col_pos + xoffsets[op]) == start_pos){
		    keep_going = false;
		    break;
		}
		// otherwise push back the points and set the offsets and stuff appropriately..
		points.push_back(twoDPoint(minCol - 1 + col_pos + xoffsets[op], minRow - 1 + row_pos + yoffsets[op]));
		row_pos = row_pos + yoffsets[op];
		col_pos = col_pos + xoffsets[op];
//		cout << "  " << col_pos << "," << row_pos;
		// and finally the important thing..
		offsetPos = offset_offsets[op];    // which should be ok.. 
		break;
	    }
	}
    }
//    cout << endl;
    nucleus.perimeter = points;
//    nucleus.smoothenPerimeter();  // turns out not to be so useful.. 
    return(nucleus);
//    return(points);
}

vector<Nucleus> imageData::currentNuclei(){
    return(nuclei);
}

bool imageData::simpleLine(float* line, unsigned int dimension, unsigned int pos1, unsigned int pos2, unsigned int waveIndex){
    // first check that waveIndex is ok.
    if(waveIndex >= (uint)w){
	cerr << "can't return a simple line with waveIndex : " << waveIndex << endl;
	return(false);
    }
    float* source = data + waveIndex * x * y * z;  // only need to do this once.
    // then we do different things depending on how things are implemented.
    if(dimension == 0){
	// an X -line at pos y, z
	if(pos1 < (uint)y && pos2 < (uint)z){
	    memcpy((void*)line, (void*)(source + pos2 * x * y + pos1 * x), sizeof(float) * x);
	    return(true);
	}
	cerr << "simpleLine corrdinates for x line are bad: " << pos1 << "\t" << pos2 << endl;
	return(false);
    }
    if(dimension == 1){
	if(pos1 >= (uint)x || pos2 >= (uint)z){
	    cerr << "simpleLine corrdinates for y line are bad: " << pos1 << "\t" << pos2 << endl;
	    return(false);
	}
	// then copy value manually..
	for(int i=0; i < y; i++){
	    line[i] = source[pos2 * x * y + i * x + pos1];
	}
	return(true);
    }
    if(dimension == 2){
	if(pos1 >= (uint)x || pos2 >= (uint)y){
	    cerr << "simpleLine corrdinates for z line are bad: " << pos1 << "\t" << pos2 << endl;
	    return(false);
	}
	for(int i=0; i < z; i++){
	    line[i] = source[i * x * y + pos2 * x + pos1];
	}
	return(true);
    }
    cerr << "imageData::simpleLine unknown dimension : " << dimension << endl;
    return(false);
}

	    
float* imageData::arbitraryLine(unsigned int wl, unsigned int x_begin, unsigned int y_begin, unsigned int z_begin, unsigned int x_end, unsigned int y_end, unsigned int z_end, unsigned int points){
    // first some reality checks.
    if(wl >= (uint)w){
	cerr << "imageData::arbitraryLine wl is out of bounds" << endl;
	return(0);
    }
    if(!(x_begin && x_end && y_begin && y_end && z_begin && z_end)){
	cerr << "imageData :: arbitrary line can't cope with edge or surface positions.. " << endl;
	return(0);
    }
    if((int)x_begin >= x-1 || (int)x_end >= x-1 || (int)y_begin >= y-1 || (int)y_end >= y-1 || (int)z_begin >= z-1 || (int)z_end >= z-1){   // -1 since we need a one voxel window
	cerr << "imageData::arbitraryLine point out of bounds" << endl;
	return(0);
    }
    if(!points){
	cerr << "arbitraryLine 0 length line requested" << endl;
	return(0);
    }
    // at which point we can actually make a line..
    float* line = new float[points];
    
    // basically take the points in the line and go through each one and make an interpolated line. Assume that each point lies at the center
    // of a virtual voxel, then sum the partial values from all surrounding voxels..
    
    // since we will be dealing with floats, lets make some float parameters..
    float xb = (float)x_begin + 0.5;
    float xe = (float)x_end + 0.5;
    float yb = (float)y_begin + 0.5;
    float ye = (float)y_end + 0.5;
    float zb = (float)z_begin + 0.5;
    float ze = (float)z_end + 0.5;           // the line goes through the centers of the voxels

//    cout << "making line from " << xb << "," << yb << "," << zb << "  --> " << xe << "," << ye << "," << ze << endl;
//    cout << "for wavelength " << wl << "  "  << wavelengths[wl] << endl;
    float* source = data + wl * x * y * z;
    for(uint i=0; i < points; i++){
	line[i] = 0;
	// first calculate the current position and the limits of the box...
	float frac = float(i)/float(points-1);        // the fraction is between 0 and 1..
	float xc = xb + frac * (xe - xb);
	float yc = yb + frac * (ye - yb);
	float zc = zb + frac * (ze - zb);
	//cout << i << "\t" << xc << "," << yc << "," << zc << endl;
	float propsum = 0;
	// this defines the new center. Any voxel whose center lies within a
	for(int mz=-1; mz < 2; mz += 2){    // m is either -1 or +1
	    for(int my=-1; my < 2; my += 2){
		for(int mx=-1; mx < 2; mx += 2){
		    // the voxel coordinates we will be adding from are then going to be :
		    int xa = int(xc + float(mx) * 0.5);
		    int ya = int(yc + float(my) * 0.5);
		    int za = int(zc + float(mz) * 0.5);
		    // and the proportion is simply the product of the partial proportions.. 
		    float prop = (1.0 - fabs((float(xa) + 0.5) - xc)) * (1.0 - fabs((float(ya) + 0.5) - yc)) * (1.0 - fabs((float(za) + 0.5) - zc)); 
		    line[i] += prop * source[za * x * y + ya * x + xa];
		    propsum += prop;
		    //cout << "\tadding : " << mx << "," << my << "," << mz << "\t voxel : " << xa << "," << ya << "," << za 
		    // << "\tvalue : " << source[za * x * y + ya * x + xa] <<  "\tprop : " << prop << "\tpropsum " << propsum << endl;
		    
		}
	    }
	}
    }
    return(line);        // that is all a bit mind screwing.. but it might actually work.. 
}
    

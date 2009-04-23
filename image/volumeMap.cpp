#include "volumeMap.h"
#include <iostream>

using namespace std;

// although this could fail due to a long overflow; we would almost
// certainly fail due to lack of memory in that case..

VolumeMap::VolumeMap(){
    cout << "Empty constructor for Volume Map , we should'nt really see this" << endl;
}

VolumeMap::VolumeMap(int width, int height, int depth){
    if(width < 0 || height < 0 || depth < 0){
	cerr << "VolumeMap constructor, negative dimension specified. That sucks" << endl;
	return;
    }

    w = width; h = height; d = depth;
    size = (ulong)w * (ulong)h * (ulong)d;
    lineNo = (uint)h * (uint)d;
    
    lines.resize(lineNo);
    mask = new VolumeMask( (ulong)w, (ulong)h, (ulong)d );
};

VolumeMap::~VolumeMap(){
    delete mask;
}

bool VolumeMap::insert(off_set pos, ptr* obj){
    int x, y, z;
    toVol(pos, x, y, z);
    return( insert(x, y, z, obj) );
}

bool VolumeMap::insert(int x, int y, int z, ptr* obj){
    if(outOfBounds(x, y, z))
	return(false);
    
    if(!mask->setMask(true, x, y, z))
	return(false);
    
    // do not use insert(make_pair()), as we want to be able to change an old value
    lines[(z * h) + y ][x] = obj; 

    return(true);
    
}

ptr* VolumeMap::value(off_set pos){
    int x, y, z;
    toVol(pos, x, y, z);
    return( value(x, y, z) );
}

ptr* VolumeMap::value(int x, int y, int z){
    if(outOfBounds(x, y, z))
	return(0);
    if(!mask->mask(x, y, z))
	return(false);
    map<int, ptr*>::iterator it = lines[(z * h) + y].find(x);
    if(it == lines[(z * h) + y].end())
	return(0);
    return(it->second);
}

void VolumeMap::clear(){
    mask->zeroMask();
    for(uint i=0; i < lines.size(); ++i){
	lines[i].clear();
    }
}

    
	

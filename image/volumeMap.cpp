#include "volumeMap.h"
#include <iostream>

using namespace std;

// although this could fail due to a long overflow; we would almost
// certainly fail due to lack of memory in that case..

VolumeMap::VolumeMap(int width, int height, int depth){
    if(width < 0 || height < 0 || depth < 0){
	cerr << "VolumeMap constructor, negative dimension specified. That sucks" << endl;
	return;
    }

    w = width; h = height; d = depth;
    size = (uint)w * (uint)h * (uint)d;
    lineNo = (uint)h * (uint)d;
    
    lines.resize(lineNo);
    mask = new VolumeMask( (ulong)w, (ulong)h, (ulong)d );
};

VolumeMap::~VolumeMap(){
    cout << "VolumeMap destructor doing nothing" << endl;
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
    
    lines[(z * h) + y ].insert(make_pair(x, obj));
    return(true);
    
}

ptr* VolumeMap::value(off_set pos){
    int x, y, z;
    toVol(pos, x, y, z);
    return( value(x, y, z) );
}

ptr* VolumeMap::value(int x, int y, int z){
    if(outOfBounds(x, y, z))
	return(false);
    map<int, ptr*>::iterator it = lines[(z * h) + y].find(x);
    if(it == lines[(z * h) + y].end())
	return((ptr*)0);
    return(it->second);
}


    
	

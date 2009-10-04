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
    for(uint i=0; i < lines.size(); ++i)
	lines[i] = new LinMap(10);
    mask = new VolumeMask( (ulong)w, (ulong)h, (ulong)d );
    linMapMask = new VolumeMask( (ulong)w, (ulong)h, (ulong)d );
};

VolumeMap::~VolumeMap(){
    delete mask;
    delete linMapMask;
    for(uint i=0; i < lines.size(); ++i)
	delete(lines[i]);
}

off_set VolumeMap::memSize(){
    cout << "\tSize of tupel\t: " << sizeof(tupel) << endl;
    cout << "\tSize of LinMap\t: " << sizeof(LinMap) << endl;
    off_set mem = 0;
    for(uint i=0; i < lines.size(); ++i)
	mem += lines[i]->memSize();
    return(mem);
}

off_set VolumeMap::mapSize(){
    off_set s = 0;
    for(uint i=0; i < lines.size(); ++i){
	s += lines[i]->mapSize();
    }
    return(s);
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
     // if the object is an empty pointer, don't waste space for it.
     if(!obj)
 	return(true);

     linMapMask->setMask(true, x, y, z);
     // do not use insert(make_pair()), as we want to be able to change an old value
     lines[(z * h) + y ]->insert(x, obj);
//    lines[(z * h) + y ][x] = obj; 
     
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
    if(!linMapMask->mask(x, y, z))
	return(0);
    
    return( lines[(z*h) + y]->value(x) );
    
//     map<int, ptr*>::iterator it = lines[(z * h) + y].find(x);
//     if(it == lines[(z * h) + y].end())
// 	return(0);
//     return(it->second);
}

void VolumeMap::clear(){
    mask->zeroMask();
    linMapMask->zeroMask();
    for(uint i=0; i < lines.size(); ++i){
	lines[i]->clear();
    }
}

    
	

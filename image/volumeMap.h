#ifndef VOLUMEMAP_H
#define VOLUMEMAP_H

#include "volumeMask.h"
#include "../dataStructs.h"
#include "linMap.h"
#include "blob.h"
#include <map>
#include <vector>
#include <iostream>

// one day I'll learn how to use templates..

//typedef blob ptr;  // this is defined in the linMap.h
typedef unsigned int uint;
typedef unsigned long ulong;

template <class T> class VolumeMap {
 public:
    VolumeMap();
    VolumeMap(int width, int height, int depth);
    ~VolumeMap();

    bool insert(o_set pos, T obj);
    bool insert(int x, int y, int z, T obj);

    // these give 0 if not masked or out of bounds.
    T value(o_set pos);
    T value(int x, int y, int z);

    bool masked(int x, int y, int z){
	return(mask->mask(x, y, z));
    }

    o_set memSize();
    o_set mapSize();

    void clear();

 private:
    // use one map for each x-line in the sample.
    // and a bitwise mask for quick checks.

//    std::vector< std::map<int, ptr*> > lines;
    std::vector< LinMap<T>*> lines;
    VolumeMask* mask;            // this mask refers the volume
    VolumeMask* linMapMask;      // this mask refers to the lines, object mask.

    unsigned long size;
    unsigned int lineNo;
    // the dimensions deliberately use signed integers.
    int w, h, d;  

    o_set linear(int x, int y, int z){
	return( (o_set)z * (o_set)w * (o_set)h + (o_set)y * (o_set)w + (o_set)x );
    }
    void toVol(o_set p, int& x, int& y, int& z){
	z = p / (w * h);
	y = (p % (w * h)) / w;
	x = p % w;
    }

    bool outOfBounds(int x, int y, int z){
	return(x >= w || y >= h || z >= d || x < 0 || y < 0 || z < 0);
    }
};


template <class T> VolumeMap<T>::VolumeMap(){
  std::cout << "Empty constructor for Volume Map , we should'nt really see this" << std::endl;
}

template <class T> VolumeMap<T>::VolumeMap(int width, int height, int depth){
    if(width < 0 || height < 0 || depth < 0){
      std::cerr << "VolumeMap constructor, negative dimension specified. That sucks" << std::endl;
	return;
    }

    w = width; h = height; d = depth;
    size = (ulong)w * (ulong)h * (ulong)d;
    lineNo = (uint)h * (uint)d;
    
    lines.resize(lineNo);
    for(uint i=0; i < lines.size(); ++i)
	lines[i] = new LinMap<T>(10);
    mask = new VolumeMask( (ulong)w, (ulong)h, (ulong)d );
    linMapMask = new VolumeMask( (ulong)w, (ulong)h, (ulong)d );
};

template <class T> VolumeMap<T>::~VolumeMap(){
    delete mask;
    delete linMapMask;
    for(uint i=0; i < lines.size(); ++i)
	delete(lines[i]);
}

template <class T> o_set VolumeMap<T>::memSize(){
  std::cout << "\tSize of tupel\t: " << sizeof(tupel<T>) << std::endl;
  std::cout << "\tSize of LinMap\t: " << sizeof(LinMap<T>) << std::endl;
  o_set mem = 0;
  for(uint i=0; i < lines.size(); ++i)
    mem += lines[i]->memSize();
  return(mem);
}

template <class T> o_set VolumeMap<T>::mapSize(){
    o_set s = 0;
    for(uint i=0; i < lines.size(); ++i){
	s += lines[i]->mapSize();
    }
    return(s);
}

template <class T> bool VolumeMap<T>::insert(o_set pos, T obj){
    int x, y, z;
    toVol(pos, x, y, z);
    return( insert(x, y, z, obj) );
}

template <class T> bool VolumeMap<T>::insert(int x, int y, int z, T obj){
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

template <class T> T VolumeMap<T>::value(o_set pos){
    int x, y, z;
    toVol(pos, x, y, z);
    return( value(x, y, z) );
}

template <class T> T VolumeMap<T>::value(int x, int y, int z){
    if(outOfBounds(x, y, z))
	return(0);
    if(!linMapMask->mask(x, y, z))
	return(0);
    
    return( lines[(z*h) + y]->value(x) );
}

template <class T> void VolumeMap<T>::clear(){
    mask->zeroMask();
    linMapMask->zeroMask();
    for(uint i=0; i < lines.size(); ++i){
	lines[i]->clear();
    }
}

    

#endif

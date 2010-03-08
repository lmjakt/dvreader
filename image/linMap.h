#ifndef LINMAP_H
#define LINMAP_H

//#include "blob.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>

// an attempt to replace a map<off_set, ptr*> with a tuple* and a binary searching
// algorithm.
// This being done to try to reduce memory use, at the expense of processing time.
// But since we can make some assumptions about behaviour it might turn out alright

// since I haven't learnt how to do templates

//#typedef blob ptr;

template <class T> struct tupel {
  tupel(){
    x = 0;
    obj = 0;
  }
  
  int x;
  T obj;
  //  ptr* obj;
};

template <class T> class LinMap {
 public:
    LinMap(uint expSize);
    LinMap();
    ~LinMap();  // deletes the tupels only.
    
    //bool insert(int pos, ptr* obj);
    bool insert(int pos, T obj);
    //ptr* value(int pos);
    T value(int pos);
    void clear();
    uint memSize();
    uint mapSize();

 private:
    uint increment;
    uint memsize;
    uint size;
    tupel<T>* tupels;

    bool findPos(int pos, uint& i);
    //    void insert(int pos, ptr* obj, uint i);
    void insert(int pos, T obj, uint i);
};


template <class T> LinMap<T>::LinMap(){
    LinMap(10);
}

template <class T> LinMap<T>::LinMap(uint expSize){
    if(expSize < 1){
      std::cerr << "LinMap consctructor does not like exp Size: " << expSize << std::endl;
	exit(1);
    }
    memsize = increment = expSize;
    size = 0;
    tupels = new tupel<T>[memsize];
}

template <class T> LinMap<T>::~LinMap(){
    delete[] tupels;
}

template <class T> void LinMap<T>::clear(){
    delete[] tupels;
    tupels = new tupel<T>[increment];
}

template <class T> uint LinMap<T>::memSize(){
    return(memsize);
}

template <class T> uint LinMap<T>::mapSize(){
    return(size);
}

template <class T> bool LinMap<T>::insert(int pos, T obj){
    // Assume that we are likely to add things on to the end..
    if(!size){
	tupels[0].obj = obj;
	tupels[0].x = pos;
	++size;
	return(true);
    }
    if(pos > tupels[size-1].x){
	if(size < memsize){
	    tupels[size].obj = obj;
	    tupels[size].x = pos;
	    ++size;
	    return(true);
	}
	insert(pos, obj, size);
	return(true);
    }
    uint i;
    if(findPos(pos, i)){
	tupels[i].x = pos;
	tupels[i].obj = obj;
	return(true);
    }
    insert(pos, obj, i);
    return(true);
}

template <class T> T LinMap<T>::value(int pos){
    T p = 0;
    uint i;
    if(!findPos(pos, i)){
//	cerr << "LinMap value() didn't find anything " << endl;
	return(p);
    }
    return(tupels[i].obj);
}

template <class T> void LinMap<T>::insert(int pos, T obj, uint i){
    if(size < memsize){
	memmove( (tupels + i + 1), (tupels + i), sizeof(tupel<T>)*(size - i) );
	tupels[i].x = pos;
	tupels[i].obj = obj;
	++size;
	return;
    }
    memsize += increment;
    tupel<T>* newTupels = new tupel<T>[memsize];
    memcpy(newTupels, tupels, sizeof(tupel<T>)*i);
    newTupels[i].x = pos;
    newTupels[i].obj = obj;
    memcpy( (newTupels + i + 1), tupels + i, sizeof(tupel<T>)*(size - i) );
    ++size;
    delete[] tupels;
    tupels = newTupels;

}

// returns false if no exact match found. In any case i is set to the correct
// memory index to use in the array
template <class T> bool LinMap<T>::findPos(int pos, uint& i){
    int p = (int)size;
    int s = p;
    int sign = -1;
    
    // oh so ugly.. 
    if(size == 1){
	if(tupels[0].x == pos){
	    i = 0;
	    return(true);
	}
	i = pos > tupels[0].x ? 1 : 0;
	return(false);
    }

    for(int den = 2; den <= s; den *= 2){
	p = p + (sign * s / den);
	if( tupels[p].x == pos){
	    i = p;
	    return(true);
	}
	if( tupels[p].x > pos && tupels[p-1].x < pos ){
	    i = p;
	    return(false);
	}
	if(tupels[p].x > pos){
	    sign = -1;
	    continue;
	}
	sign = +1;
    }
    // use the residual value of sign to go up or down.
    int tp;
    for(tp = p + sign; (tp >= 0 && tp < s); tp += sign){
	if(tupels[tp].x == pos){
	    i = tp;
	    return(true);
	}
	// otherwise a bit more difficult since we do not know the value of sign
	if( (tupels[tp].x < pos) != (tupels[tp - sign].x < pos) ){
	    i = (tp - sign) > tp ? (tp-sign) : tp;
	    return(false);
	}
    }
    // if we end up here, then tp is either -1 or equal to size.
    // so we either have to make it one more or ??. 
    // or tp is equal to the last index (special case which shouldn't happen).
    i = tp < 0 ? 0 : s;
    return(false);
}

    
#endif

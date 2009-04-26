#ifndef LINMAP_H
#define LINMAP_H

#include "blob.h"

// an attempt to replace a map<off_set, ptr*> with a tuple* and a binary searching
// algorithm.
// This being done to try to reduce memory use, at the expense of processing time.
// But since we can make some assumptions about behaviour it might turn out alright

// since I haven't learnt how to do templates

typedef blob ptr;

struct tupel {
    tupel(){
	x = 0;
	obj = 0;
    }
    
    int x;
    ptr* obj;
};

class LinMap {
 public:
    LinMap(uint expSize);
    LinMap();
    ~LinMap();  // deletes the tupels only.
    
    bool insert(int pos, ptr* obj);
    ptr* value(int pos);
    void clear();
    uint memSize();
    uint mapSize();

 private:
    uint increment;
    uint memsize;
    uint size;
    tupel* tupels;

    bool findPos(int pos, uint& i);
    void insert(int pos, ptr* obj, uint i);
};

    
#endif

#ifndef SUPERBLOB_H
#define SUPERBLOB_H

#include "blob.h"

struct SuperBlob {
    unsigned int membership;
    std::vector<id_blob> blobs;
    
    SuperBlob(){
	membership = 0;
    }
    SuperBlob(id_blob b){
	membership = b.mapper_id;
	blobs.push_back(b);
    }

    void addBlob(id_blob b){
	membership |= b.mapper_id;
	blobs.push_back(b);
    }
};

#endif

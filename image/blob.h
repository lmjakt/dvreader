#ifndef BLOB_H
#define BLOB_H

#include <vector>

typedef unsigned int off_set;
typedef unsigned int uint;

struct blob {
    blob(){
	min = max = 0;
	peakPos = 0;
	r = g = b = 200;
    }
    ~blob(){
    }
    void size(uint& s);
    std::vector<off_set> points;
    std::vector<bool> surface; 
    float min, max, sum;
    int max_x, min_x, max_y, min_y, max_z, min_z;
    unsigned long peakPos;
    int r, g, b;
};

#endif


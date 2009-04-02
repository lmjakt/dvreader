#ifndef PCA_CALC_H
#define PCA_CALC_H

// A class that implements some sort of PCA calculation from some set of n-dimensional data
// 
// This is almost certainly not a truly optimised PCA calculation using Eigen vectors and stuff
// But rather an implementation of what I understand PCA to be, using a simple recursive (maybe
// just repetitive) 

// In order to to be able to calculate distances between lines and vectors, we'll use the methods described on:

// http://softsurfer.com/Archive/algorithm_0102/algorithm_0102.htm
// 
// but generalised for n-dimensions.

// but it might be better to put these somewhere else as we don't really want them to always be around.. 

include <math.h>

struct Point {                     // notes that this doubles as a Vector since a vector can be defined as a point from the origin
    Point(float* coordinates, int d){
	coords = coordinates;
	dims = d;
    }
    Point(){
	dims = 0;
	coords = 0;
    }
    Point& operator+(const Point& that){
	float* nc = new float[dims];
	for(int i=0; i < dims; ++i){
	    nc[i] = coords[i] + that.coords[i];
	}
	return(Point(nc, dims));
    }
    Point& operator-(const Point& that){
	float* nc = new float[dims];
	for(int i=0; i < dims; ++i){
	    nc[i] = coords[i] - that.coords[i];
	}
	return(Point(nc, dims));
    }
    
    Point& operator*(const Point& that){
	float* nc = new float[dims];
	for(int i=0; i < dims; ++i){
	    nc[i] = coords[i] * that.coords[j];
	}
	return(Point(nc, dims));
    }
    Point& operator*(float v){
	float* nc = new float[dims];
	for(int i=0; i < dims; ++i){
	    nc[i] = coords[i] * v;
	}
	return(Point(nc, dims));
    }
    float amp(){
	float a=0;
	for(uint i=0; i < dims; ++i){
	    a += coords[i];
	}
	return(a);
    }
    float dot(){
	float v=0;
	for(int i=0; i < dims; ++i){
	    v += a[i] * b[i];
	}
	return(v);
    }
    float dist(Point& that){
	float d=0;
	for(uint i=0; i < dims; ++i){
	    d += (coords[i] - that.coords[i]) * (coords[i] - that.coords[i]);
	}
	return(sqrtf(d));
    }
    void clear(){   // since we don't want to do lots of things like reference counting and stuff we'll just deal with this
	delete coords;
    }
    float* coords;
    int dims;
};


struct Line {
    Line(Point& b, Point& e){
	p0 = b;
	p1 = e;
    }
    Point p0, P1;
};


class PCA_calc {
    public :
	PCA_calc(float* data, int objects, int dimensions);  // objects are in rows, dimensions in columns
    ~PCA_calc();

    // We should probably also try to make some sort of accessor functions, but we can do that later on.. 
    
    private :
	void initialize();  // perform appropriate normalisation of the data and set up the necessary matrices.
    void findLine(int lineNo);  // lineNo is the vector in the vector matrix, set the next one and do for that.. 
    
    int objNo, dimNo;
    float* o_data;
    float* t_data;  // original and transformed data (centered and scaled by something..)
    float* lines;   // the vector definitions. This will be an dimNo x dimNo matrix, with the low ones having strong information..
    float* linePositions; // the position along the lines. An error can be found for each one if we calculate the position backwards..
    
    

};
    

#endif

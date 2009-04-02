//Copyright Notice
/*
    dvReader deltavision image viewer and analysis tool
    Copyright (C) 2009  Martin Jakt
   
    This file is part of the dvReader application.
    dvReader is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
 
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
   
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

//Copyright Notice
/*
    eXintegrator integrated expression analysis system
    Copyright (C) 2004  Martin Jakt & Okada Mitsuhiro
  
    This file is part of the eXintegrator integrated expression analysis system. 
    eXintegrator is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version. 

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
  
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    PS. If you can think of a better name, please let me know...
*/
//End Copyright Notice

#ifndef DISTANCEMAPPER_H
#define DISTANCEMAPPER_H

#include <qthread.h>
#include <vector>
#include <qmutex.h>
#include <iostream>

//using namespace std;    //please forgive me..

struct componentVector {
    bool attractive;     // is it an attractor or repulsor..
    float* forces;
    int forceNo;        // this should have been called dimNo for dimensionNo, I may rename later .. 
    
    //vector<float> forces;  // the components. 
    componentVector(){
	forces = 0;
	forceNo = 0;
    };   // empty constructor
    componentVector(std::vector<float> f, bool a){
	attractive = a;
	forces = new float[f.size()];
	forceNo = f.size();
	for(int i=0; i < forceNo; i++){
	    forces[i] = f[i];
	}
	//forces = f;
    }
    componentVector(float* f, int n, bool a){
	attractive = a;
	forces = f;
	//    forces = new float[n];
	// memcpy(forces, f, n * sizeof(float));    // should be ok,,,
	forceNo = n;
    }
    void assignFrom(componentVector* cv){
	attractive = cv->attractive;
	if(cv->forceNo != forceNo){
	    delete forces;
	    forceNo = cv->forceNo;
	    forces = new float[forceNo];
	}
	memcpy((void*)forces, (void*)cv->forces, sizeof(float) * forceNo);
    }
    void reset(){
	memset((forces), 0, sizeof(float) * forceNo);
    }
    ~componentVector(){
	delete []forces;
    }
};

struct dpoint {
    
    float* coordinates;
    float* forceVectors;
    int dimNo;       // the no of dimensions. --  we have one value for each forceVector.. 
    
    componentVector** components;
    int componentNo;
    int componentSize;   // memory allocated for components.
    
    int index;                    // some kind of identifier..
    float stress;                 // total amount of stress (absolute).. 
    
    dpoint();  
    dpoint(int i, int dimensions, int compNo=100);
    ~dpoint();
    
    float adjustVectors(dpoint* p, float d, bool linear=true);   // given another point, with an optimal distance of d
    // adjust the vectors and return a measure of force in the system.. 
    float move(float forceMultiplier);            // move the point, reset the force vectors to 0 if reset is true.. return the euclidean distance moved
    void resetForces();          // set the stress and force vectors to be 0.. 
    void setPos(std::vector<float> coords);
    dpoint* copy();
    void assignTo(dpoint* point);
    
    private :
    //void addComponent(float* f, int n, bool a);
    void addComponent(componentVector* cv);    // check how much memory we have, expand if necessary, otherwise, just add.. 
    
};


// A class that spawns a thread that attempts to map a set of objects in 2d based
// on all agains all distances.. -- whilst calculating the stress in the system.. 

class DistanceMapper : public QThread
{
    public :
	DistanceMapper(std::vector<int> expI, std::vector<std::vector<float> > d, int dims,  QMutex* mutx, std::vector<dpoint*>* prntPoints, QObject* prnt, bool* drawPoints);
//    DistanceMapper(vector<int> expI, vector<vector<float> > d, int dims,  QMutex* mutx, vector<vector<dpoint*> >* prntPoints, QObject* prnt);
    ~DistanceMapper();  // don't know how much I'll need for these..
    void restart();     // start the mapping process agains. 
    void updatePosition(int i, float x, float y);  
    void initialisePoints();       // find some nice random positions for the points..
    
    bool* reDrawPoints;
    bool calculating; 
    bool initOK;        // so we can check if it initialised ok.. 
    private :
	std::vector<int> expts;    // in this case each object represents an experiment, but it could be anything, -- a gene even. !!
    std::vector<std::vector<float> > distances;  // the optimal distances. 
    std::vector<dpoint*> points;              // the points representing the objects (usually experiments.. )..  
    std::vector<dpoint*>* parentPoints;
//  vector<vector<dpoint*> >* parentPoints;
    QObject* parent;                 // for updating information.. 
    QMutex* pointMutex;
    
    float moveFactor;               // how much do we move a point by. -- i.e. how much do we multiply the forceVectors by to get a 
    // to move distance.. 
    float totalDistance;
    int dimensionality;
    // some functions..
    // don't need because the points will cheat if in the same point.. start all points at the origin.. 
    
    float adjustVectors(bool linear=true);          // adjust the Vectors in the points.. returns the total force
    float movePoints();             // move the points.. and reset force vectors.. -- return distance moved
    void resetPoints();             // just go through and reset the points.. 
    void clonePoints();
    
    protected :
	void run();                 // the main function, essentially iterates through initialise points and 
};

#endif

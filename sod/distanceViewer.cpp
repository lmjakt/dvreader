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

#include "distanceViewer.h"
#include "distanceMapper.h"
#include "pointDrawer.h"
//#include "tdPointDrawer.h"
//#include "../som/somDrawer.h"
//#include "../customWidgets/fSpinBox.h"
#include <QLayout>
#include <vector>
#include <QPushButton>
#include <QLabel>
#include <iostream>

using namespace std;

DistanceViewer::DistanceViewer(vector<int> expI, vector<vector<float> > d, QString cName, QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  cout << "Distance Viewer constructor " << endl;
  captionName = cName;  // for offspring windows.. maybe.. mainly.. 
  setCaption(cName);
  distances = d;
  experiments = expI;
  
  //  cout << "reserving points  " << endl;
  //  points.reserve(5000);
  //  cout << "points reservedd " << endl;

  drawer = new PointDrawer();
  cout << "made point drawer " << endl;
  drawer->setCaption(captionName);
  drawer->resize(500, 500);
  drawer->show();
  connect(drawer, SIGNAL(updatePosition(int, float, float)), this, SLOT(updatePosition(int, float, float)) );    // could be direct, but I want to be careful
  connect(drawer, SIGNAL(compareCells(vector<int>, vector<int>)), this, SIGNAL(compareCells(vector<int>, vector<int>)) );
  connect(drawer, SIGNAL(setCoordinates()), this, SLOT(setcoords()) );

  stressPlotter = new StressPlotter();
  stressPlotter->show();

  //  int dimNo = 4;
// mapper updates points, then 
  mapper = new DistanceMapper(experiments, distances, &pointMutex, &points, (QObject*)this, &stressValues, DistanceMapper::GRADUAL_PARALLEL);    
  
  frameTimer = new QTimer(this, "frameTimer");
  connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()) );
  
  watchTimer = new QTimer(this, "watchTimer");
  connect(watchTimer, SIGNAL(timeout()), this, SLOT(updatePoints()) );

  QLabel* sodLabel = new QLabel("Self Organising Deltoids", this, "sodLabel");

  //QPushButton* replayButton = new QPushButton("Replay", this, "replayButton");
  //connect(replayButton, SIGNAL(clicked()), this, SLOT(replay()) );

  QPushButton* continueButton = new QPushButton("Continue", this, "continueButton");
  connect(continueButton, SIGNAL(clicked()), this, SLOT(continueMapping()) );   // is this legal ?? 

  dimReductTypeBox = new QComboBox(false, this);
  dimReductTypeBox->insertItem("Starburst", (int)DistanceMapper::STARBURST);
  dimReductTypeBox->insertItem("Serial", (int)DistanceMapper::GRADUAL_SERIAL);
  dimReductTypeBox->insertItem("Parallel", (int)DistanceMapper::GRADUAL_PARALLEL);
  dimReductTypeBox->setCurrentItem((int)DistanceMapper::GRADUAL_PARALLEL);

//  QPushButton* restartButton = new QPushButton("Restart", this, "restartButton");
//  connect(restartButton, SIGNAL(clicked()), this, SLOT(restart()) );   // is this legal ??   

  QPushButton* startButton = new QPushButton("start", this, "startButton");
  connect(startButton, SIGNAL(clicked()), this, SLOT(start()) );

  QPushButton* deleteButton =  new QPushButton("Delete", this, "deleteButton");
  connect(deleteButton, SIGNAL(clicked()), this, SIGNAL(deleteMe()) );

  
  QLabel* dimLabel = new QLabel("Dim no.", this);
  QLabel* iterLabel = new QLabel("Iterations", this);
  dimSpinner = new QSpinBox(2, 10, 1, this);
  iterSpinner = new QSpinBox(20, 10000, 1, this);
  dimSpinner->setValue(4);
  iterSpinner->setValue(500);

  cout << "start button made, setting up the layout " << endl;
  // set up the layout..
  QVBoxLayout* vbox = new QVBoxLayout(this);

  QHBoxLayout* sodButtons = new QHBoxLayout();
  vbox->addWidget(sodLabel);
  vbox->addLayout(sodButtons);
//  sodButtons->addWidget(replayButton);
  sodButtons->addWidget(startButton);
  sodButtons->addWidget(continueButton);
  sodButtons->addWidget(dimReductTypeBox);
  //sodButtons->addWidget(restartButton);

  cout << "sod buttons done going for the bottom row " << endl;
  QHBoxLayout* bottomRow = new QHBoxLayout();
  vbox->addLayout(bottomRow);
  bottomRow->addWidget(deleteButton);
  bottomRow->addWidget(dimLabel);
  bottomRow->addWidget(dimSpinner);
  bottomRow->addWidget(iterLabel);
  bottomRow->addWidget(iterSpinner);
  bottomRow->addStretch();
  cout << "and the start Button added, must be the drawing of these that causes the problem " << endl;
  //  resize(500, 400);
  
  cout << "And what's now, constructor finished " << endl;
  // and start the mapper.. -- it's probably too fast, but there you go.. we could use a timer to do updates,, but hmm
}

DistanceViewer::~DistanceViewer(){
  cout << "destroying distance viewer .. hahhaha" << endl;
  mapper->wait();

  drawer->hide();
  delete drawer;
  stressPlotter->hide();
  delete stressPlotter;

  delete mapper;    // main things to delete.. 
  
  // and delete the data that we are not going to be using anymore.. 
  for(uint i=0; i < points.size(); i++){
    cout << "deleting from : " << i << endl;
    cout << "points " << i << "   size is " << points[i].size() << endl;
    for(uint j=0; j < points[i].size(); j++){
      delete points[i][j];
    }
  }
  for(uint i=0; i < localPoints.size(); ++i){
      delete localPoints[i];
  }
}

// Checks that the positions is rectangular and that it
// has as many rows as distances has. However, does not check
// that these actually are the real positions (i.e. that
// point to point distances would give this).
void DistanceViewer::setPositions(vector<vector<float> > p)
{
  // we shouldn't really set it, but if not.. 
  if(p.size() != distances.size() || !distances.size())
    return;
  if(!p[0].size())
    return;
  for(unsigned int i=0; i < p.size(); ++i){
    if(p[i].size() != p[0].size())
      return;
  }
  positions = p;
  real_dimensionality = p[0].size();  
  mapper->setInitialPoints(positions);
}

void DistanceViewer::setPointPlotType(PointDrawer::PointPlotType ppt)
{
  drawer->setPointPlotType(ppt);
}

void DistanceViewer::drawForces(bool b)
{
  drawer->drawForces(b);
}

void DistanceViewer::setPointDiameter(int d)
{
  drawer->setPointDiameter(d);
}

void DistanceViewer::start(){
    mapper->wait();
    deletePoints();
    mapper->setDim(dimSpinner->value(), iterSpinner->value(), dimReductTypeBox->currentItem());
    cout << "start called " << endl;
    followFrame = 0;
    watchTimer->start(20);  // 33 fps.. 
    mapper->start();   // for now, let's just run one of these babies.. !!
}

void DistanceViewer::deletePoints(){
    for(uint i=0; i < points.size(); ++i){
	for(uint j=0; j < points[i].size(); ++j)
	    delete points[i][j];
	points[i].resize(0);
    }
    points.resize(0);
}

// A comment .. 

void DistanceViewer::restart(){
  mapper->wait();
  // we have to delete the old points, this maybe dangerous as if the drawer were to try to draw something at this point we
  // will get a segmentation fault as the two vectors share the pointers. Need to do something about that.. perhaps..
  drawer->emptyData();           // so the drawer doesn't try to draw any of these points after they've been deleted..
  deletePoints();
  cout << "resized points to 0" << endl;
  mapper->reInitialise();
  cout << "and got thingy to init points" << endl;
  followFrame = 0;
  mapper->start();
  cout << "starting mapping process" << endl;
  watchTimer->start(20);
}

void DistanceViewer::continueMapping(){
  mapper->wait();
  mapper->start();  // is that possible ??
  watchTimer->start(20);
}

void DistanceViewer::updatePoints(){
  // we assume that by this time the points vector has been appended by the thingy..
  // so all we do is read off the last member of it..
    pointMutex.lock();    // hmm   -- stops the other thread if in the middle of this .. 
    if(!points.size()){
	pointMutex.unlock();
	return;
    }
    if(!mapper->calculating && followFrame >= (int)points.size()){
	cout << "stopping the timer.. " << endl;
	watchTimer->stop();
//	pointMutex.unlock();
//	return;
    }
    vector<dpoint*>& pointRefs = points.back();
    if((uint)followFrame < points.size()){
	pointRefs = points[followFrame];
	followFrame++;
    }
    // if localPoints is defined, but it's dimNo is smaller than the current one, we'll need to get rid of it and make a new one..
    if(localPoints.size() && pointRefs.size() && localPoints[0]->dimNo < pointRefs[0]->dimNo){
	cout << "Deleting localPoints and resetting " << endl;
	for(uint i=0; i < localPoints.size(); ++i)
	    delete(localPoints[i]);
	localPoints.resize(0);
    }    

    // and then we may need to make a copy..
    if(!localPoints.size()){
	localPoints.resize(pointRefs.size());
	for(uint i=0; i < localPoints.size(); ++i){
	    localPoints[i] = pointRefs[i]->copy(true);
	}
    }else{
	for(uint i=0; i < localPoints.size(); ++i){
	    pointRefs[i]->assignValues(localPoints[i]);
	}
    }
    pointMutex.unlock();   // slows down calculation in favour of drawing.. ?? good or bad, I'm not sure.. 
    drawer->setData(localPoints);

    stressPlotter->setData(stressValues);
}

void DistanceViewer::replay(){
  cout << "replay function " << endl;
  frame = 0;
  //somDrawer->setMaxDev(maxDev->fvalue());
  frameTimer->start(10);    // 25 fps.. as not so many frames..
}

void DistanceViewer::updateFrame(){
  cout << "calling updateFrame frame is : " << frame << endl;
  if(frame >= (int)points.size()){
    frameTimer->stop();
    return;
  }
  pointMutex.lock();
  vector<dpoint*> localPoints = points[frame];
  pointMutex.unlock();

  drawer->setData(localPoints);
  frame++;
}

void DistanceViewer::updatePosition(int i, float x, float y){
  mapper->updatePosition(i, x, y);
  pointMutex.lock();
  vector<dpoint*> localPoints = points.back();
  pointMutex.unlock();


  drawer->setData(localPoints);
}

void DistanceViewer::setcoords(){
  cout << "Distnace viewer set coordinates .. " << endl; 
  vector<PointCoordinate> cords;   // just push it back..
  vector<dpoint*> pts = points[points.size() -1];
  for(uint i=0; i < pts.size(); i++){        
    cords.push_back(PointCoordinate(pts[i]->index, pts[i]->coordinates, pts[i]->dimNo));
  }
  emit setCoordinates(cords);   // have these set in the clientWindow, and then have drawing things use a pointer.. to something.. 
}

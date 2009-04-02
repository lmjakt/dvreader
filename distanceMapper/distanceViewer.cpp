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

#include "distanceViewer.h"
#include "distanceMapper.h"
#include "pointDrawer.h"
#include "tdPointDrawer.h"
//#include "../som/somDrawer.h"
#include "../customWidgets/fSpinBox.h"
#include <qwidget.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QCustomEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include <qmutex.h>
#include <qpushbutton.h>
#include <qlabel.h>
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

  reDrawPoints = false;


  drawer = new PointDrawer();
  cout << "made point drawer " << endl;
  drawer->setCaption(captionName);
  drawer->resize(500, 500);
  drawer->show();
  connect(drawer, SIGNAL(updatePosition(int, float, float)), this, SLOT(updatePosition(int, float, float)) );    // could be direct, but I want to be careful
  connect(drawer, SIGNAL(compareCells(vector<int>, vector<int>)), this, SIGNAL(compareCells(vector<int>, vector<int>)) );
  connect(drawer, SIGNAL(setCoordinates()), this, SLOT(setcoords()) );


  //tdDrawer = new TDPointDrawer(this, "tdDrawer");
  int mappingDimensions = 3;
  cout << "making mapper " << endl;
  mapper = new DistanceMapper(experiments, distances, mappingDimensions, &pointMutex, &points, (QObject*)this, &reDrawPoints);    // mapper updates points, then 
  cout << "mapper made " << endl;
  
  frameTimer = new QTimer(this, "frameTimer");
  connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()) );
  
  cout << "frame timer made " << endl;

  watchTimer = new QTimer(this, "watchTimer");
  connect(watchTimer, SIGNAL(timeout()), this, SLOT(updatePoints()) );

  cout << "watchTimer made " << endl;

  QLabel* sodLabel = new QLabel("Self Organising Deltoids", this, "sodLabel");

  QPushButton* replayButton = new QPushButton("Replay", this, "replayButton");
  connect(replayButton, SIGNAL(clicked()), this, SLOT(replay()) );
  cout << "replay button made " << endl;

  QPushButton* continueButton = new QPushButton("Continue", this, "continueButton");
  connect(continueButton, SIGNAL(clicked()), this, SLOT(continueMapping()) );   // is this legal ?? 

  QPushButton* restartButton = new QPushButton("Restart", this, "restartButton");
  connect(restartButton, SIGNAL(clicked()), this, SLOT(restart()) );   // is this legal ??   

  QPushButton* startButton = new QPushButton("start", this, "startButton");
  connect(startButton, SIGNAL(clicked()), this, SLOT(start()) );

  QPushButton* deleteButton =  new QPushButton("Delete", this, "deleteButton");
  connect(deleteButton, SIGNAL(clicked()), this, SIGNAL(deleteMe()) );

  cout << "start button made, setting up the layout " << endl;
  // set up the layout..
  QVBoxLayout* vbox = new QVBoxLayout(this);
  //  vbox->addWidget(drawer);    // and more later on..
  //vbox->addWidget(tdDrawer);    // and more later on..
  QHBoxLayout* sodButtons = new QHBoxLayout();
  vbox->addWidget(sodLabel);
  vbox->addLayout(sodButtons);
  sodButtons->addWidget(replayButton);
  sodButtons->addWidget(continueButton);
  sodButtons->addWidget(restartButton);

  cout << "sod buttons done going for the bottom row " << endl;
  QHBoxLayout* bottomRow = new QHBoxLayout();
  vbox->addLayout(bottomRow);
  bottomRow->addWidget(deleteButton);
  bottomRow->addStretch();
  bottomRow->addWidget(startButton);
  cout << "and the start Button added, must be the drawing of these that causes the problem " << endl;
  
  cout << "And what's now, constructor finished " << endl;
  // and start the mapper.. -- it's probably too fast, but there you go.. we could use a timer to do updates,, but hmm
}

DistanceViewer::~DistanceViewer(){
  cout << "destroying distance viewer .. hahhaha" << endl;
  mapper->wait();

  drawer->hide();
  delete drawer;

  delete mapper;    // main things to delete.. 
  
  // and delete the data that we are not going to be using anymore.. 
//   for(int i=0; i < points.size(); i++){
//     cout << "deleting from : " << i << endl;
//     cout << "points " << i << "   size is " << points[i].size() << endl;
  for(int j=0; j < points.size(); j++){
      delete points[j];
  }
//  }
}

void DistanceViewer::start(){
  cout << "start called " << endl;
  followFrame = 0;
  //somFrame = 0; 
  watchTimer->start(20);  // 33 fps.. 
  mapper->start();   // for now, let's just run one of these babies.. !!
}

void DistanceViewer::restart(){
  mapper->wait();
  // we have to delete the old points, this maybe dangerous as if the drawer were to try to draw something at this point we
  // will get a segmentation fault as the two vectors share the pointers. Need to do something about that.. perhaps..
  drawer->emptyData();           // so the drawer doesn't try to draw any of these points after they've been deleted..
//   for(int i=0; i < points.size(); i++){
//     cout << "deleting from : " << i << endl;
//     for(int j=0; j < points[i].size(); j++){
//       delete points[i][j];
//     }
//   }
//  points.resize(0);    // forget the old points, we could store them somehwere, but maybe not.. 
  mapper->initialisePoints();
  followFrame = 0;
  mapper->start();
  watchTimer->start(20);
}

void DistanceViewer::continueMapping(){
  mapper->wait();
  mapper->start();  // is that possible ??
  watchTimer->start(20);
}



void DistanceViewer::customEvent(QCustomEvent* e){
  if(e->type() == 25341){
    updatePoints();
  }else{
    cout << "unknown QCustomEvent type : " << e->type() << endl;
  }
}

void DistanceViewer::updatePoints(){
  // we assume that by this time the points vector has been appended by the thingy..
  // so all we do is read off the last member of it..
  pointMutex.lock();    // hmm   -- stops the other thread if in the middle of this .. 
  if(!mapper->calculating && !reDrawPoints){
//  if(!mapper->calculating && followFrame >= points.size()){
    cout << "stopping the timer.. " << endl;
    watchTimer->stop();
    pointMutex.unlock();
    return;
  }
  
  cout << "calling setData followFrame is : " << followFrame << "  and points size is : " << points.size() << endl;
//  if(points.size() <= followFrame){
  if(!reDrawPoints){
    cerr << "reDrawPoints is false" << endl;
    pointMutex.unlock();
    return;
  }
//  vector<dpoint*> localPoints = points[followFrame];
  drawer->setData(points);
  reDrawPoints = false;
  followFrame++;
  // it is necessary to keep the mutex locked during the drawing since we do not make another copy of the 
  // data. We could do this, and then call draw, and that might be favourable. Given the assignTo dpoint
  // function this should be easy and should be possible to do without any particular problem.. 
  pointMutex.unlock();   // slows down calculation in favour of drawing.. ?? good or bad, I'm not sure.. 

//  drawer->setData(localPoints);
}



void DistanceViewer::replay(){
  cout << "replay function " << endl;
  frame = 0;
  //somDrawer->setMaxDev(maxDev->fvalue());
  frameTimer->start(10);    // 25 fps.. as not so many frames..
}

// void DistanceViewer::replayMap(){
//   somFrame = 0;
//   somTimer->start(250);   // 4 frames per second..
// }

void DistanceViewer::updateFrame(){
//   cout << "calling updateFrame frame is : " << frame << endl;
//   if(frame >= points.size()){
     frameTimer->stop();
     cout << "Replay function is removed from this version to save memory.. " << endl;
//     return;
//   }
//   pointMutex.lock();
//   vector<dpoint*> localPoints = points[frame];
//   pointMutex.unlock();

//   drawer->setData(localPoints);


//   //tdDrawer->setModel(localPoints);
//   frame++;
}

void DistanceViewer::updatePosition(int i, float x, float y){
    if((uint)i < points.size() && points[i]->dimNo > 1){
	mapper->wait();
	mapper->updatePosition(i, x, y);
//    pointMutex.lock();  // we don't need to use the mutex since the thread is no longer running.. 
//    vector<dpoint*> localPoints = points.back();
//    pointMutex.unlock();
	drawer->setData(points);
    }
    
  //  mapper->start();
  // watchTimer->start(30);
}

// void DistanceViewer::setcoords(){
//   cout << "Distnace viewer set coordinates .. " << endl; 
//   vector<PointCoordinate> cords;   // just push it back..
//   //  vector<dpoint> pts = points.last();     // just the most recent .. -- I forget the function that does this.. 
//   vector<dpoint*> pts = points[points.size() -1];
//   for(uint i=0; i < pts.size(); i++){        
//     cords.push_back(PointCoordinate(pts[i]->index, pts[i]->coordinates, pts[i]->dimNo));
//   }
//   emit setCoordinates(cords);   // have these set in the clientWindow, and then have drawing things use a pointer.. to something.. 
// }

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
#ifdef Q_OS_MACX
#include <limits.h>
#include <float.h>
#define MINFLOAT (FLT_MIN)
#define MAXFLOAT (FLT_MAX)
#else
#include <values.h>
#endif

using namespace std;

DistanceViewer::DistanceViewer(vector<int> expI, vector<vector<float> > d, QString cName, bool trackCoordinates, QWidget* parent, const char* name)
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
  mapper = new DistanceMapper(experiments, distances, &pointMutex, &points, (QObject*)this, &stressValues, DistanceMapper::GRADUAL_PARALLEL, trackCoordinates);    
  connect(mapper, SIGNAL(started()), this, SLOT(mapperStarted()) );
  connect(mapper, SIGNAL(finished()), this, SLOT(mapperFinished()) );

  //frameTimer = new QTimer(this, "frameTimer");
  //connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()) );
  
  watchTimer = new QTimer(this, "watchTimer");
  draw_interval = 50; // in milli seconds.. 
  connect(watchTimer, SIGNAL(timeout()), this, SLOT(updatePoints()) );

  QLabel* sodLabel = new QLabel("Self Organising Deltoids", this, "sodLabel");

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
  dimSpinner = new QSpinBox(2, 50, 1, this);
  iterSpinner = new QSpinBox(1, 10000, 1, this);
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
    for(uint j=0; j < points[i].size(); j++){
      delete points[i][j];
    }
  }
  for(uint i=0; i < localPoints.size(); ++i){
      delete localPoints[i];
  }
  for(uint i=0; i < gridPoints.size(); ++i)
    delete gridPoints[i];
}

// Checks that the positions is rectangular and that it
// has as many rows as distances has. However, does not check
// that these actually are the real positions (i.e. that
// point to point distances would give this).
void DistanceViewer::setPositions(vector<vector<float> > p, unsigned int grid_points)
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
  mapper->setInitialPoints(positions, grid_points);
}

void DistanceViewer::setAnnotation(Annotation annot){
  if(annot.n_size() == distances.size()){
    annotation = annot;
    drawer->setAnnotation(annot);
  }
}

void DistanceViewer::setDrawInterval(unsigned int di)
{
  draw_interval = di;
}

void DistanceViewer::setPointPlotType(PointDrawer::PointPlotType ppt)
{
  drawer->setPointPlotType(ppt);
}

void DistanceViewer::drawForces(bool b)
{
  drawer->drawForces(b);
}

void DistanceViewer::drawIds(bool b)
{
  drawer->drawIds(b);
}

void DistanceViewer::setPlotScale(float scale)
{
  drawer->setPlotScale(scale);
}

void DistanceViewer::setPointDiameter(int d)
{
  drawer->setPointDiameter(d);
}

void DistanceViewer::plotByAnnotationField(QString field)
{
  drawer->plotAnnotationField(field);
}

void DistanceViewer::set_simple_gaussian_background(std::vector<unsigned int> dims,
						    unsigned char* color_matrix, float var)
{
  drawer->set_simple_gaussian_background(dims, color_matrix, var);
}

void DistanceViewer::postscript(QString fname, float w, float h, bool stress)
{
  if(!stress){
    drawer->postscript(fname, w, h);
    return;
  }
  stressPlotter->postscript(fname, w, h);
}

void DistanceViewer::svg(QString fname, int w, int h)
{
  drawer->svg(fname, w, h);
}

void DistanceViewer::set_starting_dimensionality(unsigned int dim)
{
  dimSpinner->setValue(dim);
  mapper->setDim(dimSpinner->value(), iterSpinner->value(), dimReductTypeBox->currentItem());
}

void DistanceViewer::setMoveFactor(float mf)
{
  mapper->setMoveFactor(mf);
}

void DistanceViewer::setSubset(unsigned int subset_size)
{
  mapper->setSubset(subset_size);
}

void DistanceViewer::setUpdateInterval(unsigned int ui)
{
  mapper->setUpdateInterval(ui);
}

void DistanceViewer::makeTriangles()
{
  mapper->makeTriangles();
  drawer->update();
}

void DistanceViewer::setThreadNumber(unsigned int tno)
{
  mapper->setThreadNumber(tno);
}

void DistanceViewer::setGrid(bool drawGrid)
{
  mapper->wait();
  gridPoints = mapper->grid();
  std::vector<dpoint*> emptyGrid;
  if(!drawGrid){
    drawer->setGrid(emptyGrid);
  }else{
    drawer->setGrid(gridPoints);
  }
  drawer->update();
}

void DistanceViewer::setStressRange(float min, float max)
{
  stressPlotter->setStressRange(min, max);
}

void DistanceViewer::resetStressRange()
{
  stressPlotter->resetStressRange();
}

vector<float> DistanceViewer::runMultiple(unsigned int rep_no, unsigned int iter_no, unsigned int start_dim_no)
{
  mapper->wait();
  vector<float> minStresses;
  vector<float> lastStresses;
  minStresses.reserve(rep_no);
  lastStresses.reserve(rep_no);
  for(unsigned int i=0; i < rep_no; ++i){
    deletePoints();
    mapper->setDim(start_dim_no, iter_no, dimReductTypeBox->currentItem());  // reinitialises the points
    mapper->start();
    mapper->wait();
    // find the smallest 
    float minStress = MAXFLOAT;
    for(unsigned int i=0; i < stressValues.size(); ++i){
      if(stressValues[i].dimensionality() == 2.0){
	minStress = minStress > stressValues[i].stress ? stressValues[i].stress : minStress;
      }
    }
    minStresses.push_back(minStress);
    lastStresses.push_back(stressValues.back().stress);
  }
  // then output the minStresses somehow.. 
  for(unsigned int i=0; i < minStresses.size(); ++i)
    cout << i << "\t" << minStresses[i] << "\t" << lastStresses[i] << endl;
  return(minStresses);
}

void DistanceViewer::start(){
    mapper->wait();
    deletePoints();
    mapper->setDim(dimSpinner->value(), iterSpinner->value(), dimReductTypeBox->currentItem());
    cout << "start called " << endl;
    followFrame = 0;
    watchTimer->start(draw_interval);  
    mapper->start();   // for now, let's just run one of these babies.. !!
}

void DistanceViewer::mapperStarted()
{
  stopWatch.start();
}

void DistanceViewer::mapperFinished()
{
  int msec = stopWatch.elapsed();
  cout << "Distance Mapper finished elapsed time : " << msec << " ms\t" << msec / 1000 << "  sec\t" 
       << msec / (1000 * 60) << " min\t" << endl;
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

// void DistanceViewer::restart(){
//   mapper->wait();
//   // we have to delete the old points, this maybe dangerous as if the drawer were to try to draw something at this point we
//   // will get a segmentation fault as the two vectors share the pointers. Need to do something about that.. perhaps..
//   drawer->emptyData();           // so the drawer doesn't try to draw any of these points after they've been deleted..
//   deletePoints();
//   cout << "resized points to 0" << endl;
//   mapper->reInitialise();
//   cout << "and got thingy to init points" << endl;
//   followFrame = 0;
//   mapper->start();
//   cout << "starting mapping process" << endl;
//   watchTimer->start(20);
// }

void DistanceViewer::continueMapping(){
  mapper->wait();
  mapper->setIter(iterSpinner->value());
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
	gridPoints = mapper->grid();
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

// void DistanceViewer::replay(){
//   cout << "replay function " << endl;
//   frame = 0;
//   //somDrawer->setMaxDev(maxDev->fvalue());
//   frameTimer->start(10);    // 25 fps.. as not so many frames..
// }

// void DistanceViewer::updateFrame(){
//   cout << "calling updateFrame frame is : " << frame << endl;
//   if(frame >= (int)points.size()){
//     frameTimer->stop();
//     return;
//   }
//   pointMutex.lock();
//   vector<dpoint*> localPoints = points[frame];
//   pointMutex.unlock();

//   drawer->setData(localPoints);
//   frame++;
// }

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

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

#ifndef DISTANCEVIEWER_H
#define DISTANCEVIEWER_H

#include "distanceMapper.h"
#include "pointDrawer.h"
#include "stressPlotter.h"
#include "Annotation.h"
#include "../dataStructs.h"
//#include "../som/somProcess.h"
//#include "../som/somDrawer.h"
#include "../customWidgets/fSpinBox.h"
#include <QEvent>
#include <QMutex>
#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QString>
#include <QSpinBox>
#include <QComboBox>
#include <vector>


class DistanceViewer : public QWidget
{
  Q_OBJECT
    public :
  DistanceViewer(std::vector<int> expI, std::vector<std::vector<float> > d, QString cName, bool trackCoordinates, QWidget* parent=0, const char* name=0);
  ~DistanceViewer();
  
  void setPositions(std::vector<std::vector<float> > p, unsigned int grid_points=0);
  void setAnnotation(Annotation annot);
  void setDrawInterval(unsigned int di);
  void setPointPlotType(PointDrawer::PointPlotType ppt);
  void drawForces(bool b);
  void drawIds(bool b);
  void setPlotScale(float scale);
  void setPointDiameter(int d);
  void plotByAnnotationField(QString field);
  void set_simple_gaussian_background(std::vector<unsigned int> dims,
				      unsigned char* color_matrix, float var);
  void postscript(QString fname, float w, float h, bool stress=false);
  void svg(QString fname, int w, int h);
  void set_starting_dimensionality(unsigned int dim);
  void setMoveFactor(float mf);
  void setSubset(unsigned int subset_size);
  void setUpdateInterval(unsigned int ui);
  void makeTriangles();
  void setThreadNumber(unsigned int tn);
  void setGrid(bool drawGrid);

  void setStressRange(float min, float max);
  void resetStressRange();

  // returns the minStresses from each run. (Doesn't return positions, just the stress).
  std::vector<float> runMultiple(unsigned int rep_no, unsigned int iter_no, unsigned int start_dim_no);

  private slots :
  void start();    // start the mapper.. 
  void mapperStarted();
  void mapperFinished();  // just report the time taken.
  void continueMapping();                  // continue the mapping.. -- just calls start..  
  void updatePosition(int i, float x, float y);   // update the position of thingy indexed by i, to x and y.. see how this goes.. 
  void updatePoints();                     // send a vector to the thingy.. 
  void setcoords();                        // work out the coordinates and send them up the food chain.. 

  private :

  void deletePoints();                     // remove the points.. 

  QString captionName;                    // something for the captions.. 
  std::vector<std::vector<dpoint*> > points;    // the mapper thread has a pointer to this, and adds on if necessary

  std::vector<dpoint*> localPoints;             // a copy of the points that is never touched by the mapper.
  std::vector<dpoint*> gridPoints;              // shared with mapper, but deleted here. not much used yet
  std::vector<stressInfo> stressValues;         // to monitor the reduction in error... 
  std::vector<std::vector<float> > distances;

  unsigned int real_dimensionality;             // the real positions of the points.
  std::vector<std::vector<float> > positions;  // an optional vector of the original n-dimensional points.
                                               // can be used to set initial points as well to change the
                                               // the display of the points.
  Annotation annotation;
  std::vector<int> experiments;                 // might as well keep a copy in here.. it's easy enough.. 
  QMutex pointMutex;                       // lock this to access the points above.. 
  QMutex somMutex;

  DistanceMapper* mapper;                  // runs the thread and stuff....... hoo hoo yeahh.
  PointDrawer* drawer;                     // draws the point, and so far is the only thing that gets displayed. 
  StressPlotter* stressPlotter;            // plots the reduction in stress.. 
  //  TDPointDrawer* tdDrawer;                 // 3 dimensions.. !! 

  //QTimer* frameTimer;
  QTimer* watchTimer;        // check if I have any more frames.. start when starting.. 
  unsigned int draw_interval;
  QTime stopWatch;        // to check running times. Hence stopWatch.. 
  int frame;    // which frame am I t0..
  int followFrame;   // I'm following frames.. 

  QSpinBox* dimSpinner;
  QSpinBox* iterSpinner;
  QComboBox* dimReductTypeBox;
  signals :
    void compareCells(std::vector<int>, std::vector<int>);
  void setCoordinates(std::vector<PointCoordinate>);
  void deleteMe();
};

#endif



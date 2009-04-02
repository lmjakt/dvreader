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

#ifndef COMPARECONTROLLER_H
#define COMPARECONTROLLER_H

#include <qwidget.h>
#include <qlayout.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include "distanceViewer.h"
#include <QDoubleSpinBox>
#include "../customWidgets/fSpinBox.h"
#include "../dataStructs.h"
//#include "traceViewer.h"
#include <set>
#include <vector>

using namespace std;


class CompareController : public QWidget
{

  Q_OBJECT
    
    public :
    CompareController(QWidget* parent=0, const char* name=0);
  ~CompareController();

  public slots :
  void newDistances(objectDistanceInfo info);  
//  void newTrace(vector<tracePoint> points);   // receive the trace points and do something with them.. 
  private slots :
    void compareFull();
  void compareFlat();
  void readPhylipDistances();              // Just need one button.. get a file name using a file dialog thingy.. 
  void deleteSender();
//  void trace();

  private :
    QVBoxLayout* fullBox;
  QVBoxLayout* flatBox;
  QVBoxLayout* phylipBox;
  QVBoxLayout* traceBox;
  std::set<DistanceViewer*> fullDists;
  std::set<DistanceViewer*> flatDists;
  std::set<DistanceViewer*> phylipDists;
//  std::set<TraceViewer*> traces;
 
  // I need a couple of FSpinBoxes for float values.. -- these are not
  // so great at the moment so use carefully..

//  FSpinBox* flatSigma;
//  FSpinBox* flatOrder;

  QDoubleSpinBox* flatSigma;
  QDoubleSpinBox* flatOrder;

//  FSpinBox* traceSigma;
  
  QCheckBox* makeRecord;
  void recordDistances(objectDistanceInfo& info);     // write a file of the distances that can be read by something like gnuplot.. Not sure about the labels yet.. but wait for it.

  signals :
    void doFullCompare();
  void doFlatCompare(float, float);    // the parameters for a successful thing.
//  void doFlatCompare(float sigma, float order);    // the parameters for a successful thing.
  void compareCells(vector<int>, vector<int>);     // well, get the statisticial contributors to the distances.. -- have to think of some sort of thingy for this.. 
  void setCoordinates(vector<PointCoordinate>);
//  void traceExperiments(float);                        // maybe put a float in there later on.. --> for controlling things.. 
};

#endif


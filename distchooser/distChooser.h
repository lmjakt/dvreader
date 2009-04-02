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

#ifndef DISTCHOOSER_H
#define DISTCHOOSER_H

// A widget for displaying and choosing ranges from distributions of values. 

#include <qwidget.h>
#include <qrect.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>
#include <string>
#include <vector>

using namespace std;

class DistChooser : public QWidget
{
  Q_OBJECT
    
    public:
  DistChooser(string sName, int ds, QWidget* parent=0, const char* name=0);
  ~DistChooser();
  int currentAxis();    // returns the current axis.. 
  float tMax();
  float tMin();         // returns the thresholds.. 
  float vMax();         // the maximum value as set..
  float vMin();
  float dMax(){         // dMax and dMin give the display mins and maxes.. 
      return(max);
  }        
  float dMin(){
      return(min);
  }
  string statname(){
      return(statisticName);
  }
  public slots:
    //    void setData(vector<int> ind, vector<float> vf);    // set the data and plot..
    //    void setData(vector<float> vf);                       // make the indices all the same..
  void setData(vector<float> vf, float minV, float maxV, bool updateThresholds=true);
  //void setLog(bool useLog);                   // plot log or 
  void setAxis(int as);   // sets the current axis..  (if between 0 and 3);
  void setParams(float dmin, float dmax, float tmin, float tmax);
/*   bool isLog(){ */
/*     return(logValues); */
/*   } */

  protected :
    void paintEvent(QPaintEvent *e);   // for the update and draw functions..
  void mousePressEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);       // emit update signals only after.. 
  void mouseDoubleClickEvent(QMouseEvent* e);   // switch between log and linear mode.. 

  private :
    void setData();          // internal for updating after changes to divs.. 
  vector<float> values;    // I don't think I need this, we can probably remove it later. but have a feeling it might be good to be able to call it. 
  //  vector<int> indices;     // the indices
  vector<int> counts;        // the distribution !
  //vector<int> logCounts;     // the log distribution !
  int lastX;
  int lastY;          // for tracking mouse movement.
  int divs;           // the number of divisions in the distribution.. hmm. 
  int maxCount;       // good to know for drawing.. 
  float lowT, highT;    // the thresholds.. hmm as fraction of range,, -which then is fraction of window size so it maps easily..  
  float minTValue, maxTValue;   // the real threshold values.. 
  float min, max;       // the maximum and minimum values.. 
  float Min, Max;       // the min and max of the distribution.. 
  bool isZoomed;
  //bool logValues;
  string statisticName;   // the name of the statistic.. !
  bool rangesChanged;     // if the ranges changed, then emit newRanges.. in the mouseRelease.. thingy.. 
  bool showText;          // changes if the text is drawn or not..
  int axisState;          // 0 - no axis, 1 X, 2, Y, 3, Z    hmm and some buttons for changing.. !!hmm 
  QRect textArea;         // an area for the text button.. 
  QRect xArea;
  QRect yArea;
  QRect zArea;
  QRect zoomArea;        // zoom to current threshold values..
  QRect restoreArea;     // go back to Min and Max.. 

  // some margins..
  int lm;
  int rm;
  int tm;
  int bm;

  // and a popup menu..
  Q3PopupMenu* menu;
  bool mouseMoved;        // ugly, but .. 

  signals :
    void newRanges(float, float);      // max and minimum values.. !
  void axisWish(int);                  // I want this axis, oh yes I do !! };
  void copyRanges();
  void pasteRanges();
  void saveRanges();
  void readRangesFromFile();
  //void setLogValues(bool);                // change to log values.. 

  private slots :
    void saveValues();                // save the values and indices to a file.. 
  void zoomToSelection();
  void zoomOut(); 

};
#endif


    
  

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

#ifndef POINTDRAWER_H
#define POINTDRAWER_H

#include "distanceMapper.h"
#include "posInfo.h"
#include "Annotation.h"
#include <QWidget>
#include <QRect>
#include <QRectF>
#include <QPoint>
#include <QImage>
#include <QColor>
//#include <qpopupmenu.h>
#include <QString>
#include <vector>
#include <set>
#include <QPen>
#include <QPoint>

//using namespace std;
class QMenu;
class QMouseEvent;
class BackgroundDrawer;
class QRubberBand;

class PointDrawer : public QWidget
{
  Q_OBJECT

    public :
  enum PointPlotType {
    STRESS, LEVELS_PIE, ANNOT, DOTS
  };

  PointDrawer(QWidget* parent=0, const char* name=0);     // it just draws things.. so no need.. 
  ~PointDrawer();
  
  void setData(std::vector<dpoint*> p);        // set the data, then redraw..
  void emptyData();                       // empty the points.. 
  
  // how to draw the points..
  void drawForces(bool b);
  void drawIds(bool b);
  void setPointPlotType(PointPlotType ppt);
  void setPointDiameter(unsigned int d);
  void setPlotScale(float s);
  void setAnnotation(Annotation annot);
  void plotAnnotationField(QString field);
  void setPointFilter(QString filter_field, std::set<float> filter_values, bool filter_inverse);

  void postscript(QString fname, float w, float h); // in points. No resolution specified.
  void svg(QString fname, int w, int h);

  // background images..
  void set_simple_gaussian_background(std::vector<unsigned int> dims, 
				      unsigned char* color_matrix, float var);


  // n-dimensional grid
  void setGrid(std::vector<dpoint*> grid);

  private slots :
  void compareCellTypes();
  void setcoords();
  void toggleRecording();
  
  private :
    void paintEvent(QPaintEvent* e);
    void drawPicture(QPainter& p);
  void mousePressEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  void checkSelected();

  void drawPoint(QPainter& p, dpoint* point, int x, int y, QColor color);
  void drawPie(QPainter& p, dpoint* point, int x, int y, QString label);
  std::vector<QPoint> makeDiskOffsets();
  void drawDots(QPainter& p, dpoint* point, int x, int y, std::vector<QPoint>& offsets);
  void drawConnections(QPainter& p, dpoint* point);
  void drawGridPoint(QPainter& p, dpoint* gpoint);
  void determine_coordinate_scale();

  bool filterPoint(unsigned int i);

  // optional background
  QImage bg_image;
  unsigned char* bg_data; // since bg_image doesn't delete.. 
  BackgroundDrawer* bg_drawer;

  // and somewhere to keep data for stuff in..
  std::vector<QPoint> selectPoints;
  std::vector<dpoint*> points;
  std::vector<dpoint*> gridPoints; // no ownership
  std::vector<QRect> regions;  // one region for each point.. so that we can check for mouse Events... 
  std::set<uint> selectedA;
  std::set<uint> selectedB;     // bit ugly but there you go.. 
  bool itsA;              // stupid if I ever saw something but there you go.. 
  QRect movingRect;    // 0 if nothing moving, otherwise, move it by 
  int movingId;
  QRect zoomRect;
  QPoint zoomOrigin;
  QRubberBand* rubberBand;
  float maxStress;
  PosInfo pos;         // contains dimensions and converters.. 
  QRectF point_ranges; // the ranges.. 
  float drawScale;     // default to one, but can be set.
  int diameter;                               // how big do we make the circles.. 
  int margin;                                 // how much of a margin do we want.
  bool draw_forces;
  bool draw_ids;
  PointPlotType point_plot_type;
  Annotation annotation;
  QString annotation_field;
  /// the below values would be better to pack into some sort of struct.
  QString annotation_filter_field;
  std::set<float> annotation_filter_values;
  bool annotation_filter_inverse;

  std::vector<QColor> defaultColors;
  float coord_sum_max;
  float coord_radius_factor;  // used to scale the radius on the basis of coord_sum 
  
  QPen labelPen;

  int lastX, lastY;      // so that I can use QRegion::translate .. 

  QMenu* menu;

  // recording movies.. well images that can be recorded..  ?
  QString dirName;
  uint frameCounter;   // start counting from 1. 0 indicates don't record.. 

  signals :
    void updatePosition(int, float, float);  // id, x, y, .... 
  void compareCells(std::vector<int>, std::vector<int>);
  void setCoordinates();             // ask my ower to do something about the coordinates .. pass them up the food chain please..
};

#endif

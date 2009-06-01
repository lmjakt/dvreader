#ifndef SCATTERPLOTTER_H
#define SCATTERPLOTTER_H

#include <QWidget>
#include <QColor>
#include <QKeyEvent>
#include <QPainterPath>
#include <QPointF>
#include <vector>

class ScatterPlotter : public QWidget
{
  Q_OBJECT
    
    public:
  ScatterPlotter(QWidget* parent=0);
  
  bool setData(std::vector<std::vector<float> > xv, std::vector<std::vector<float> > yv, std::vector<QColor> c);

  public slots:
  void toggleLog();
  void setAlpha(int a);
  void setSelection(std::vector<bool> b);
  std::vector<std::vector<bool > > selectPoints(bool filter);

 private :
  bool initData();
  void clearData();
  void setLogValues();
  void paintEvent(QPaintEvent* e);
  void keyPressEvent(QKeyEvent* e);
  void mousePressEvent(QMouseEvent* e);
  void mouseMoveEvent(QMouseEvent* e);
  void mouseReleaseEvent(QMouseEvent* e);
  void mouseDoubleClickEvent(QMouseEvent* e);
  QPointF transformPos(QPointF p);
  QPointF valueToPlotCoordinates(float x, float y, float w, float h); // this modifies x and y and returns the point
  void valueToPlotCoordinatesR(float& x, float& y, float w, float h); // this modifies x and y and returns the point
  //QPointF logTransformPos(QPointF p);  // This is problematic. Leave it until later.

  //////// variables.. 
  std::vector<std::vector<float> > lin_x;
  std::vector<std::vector<float> > lin_y;
  std::vector<std::vector<float> > log_x;
  std::vector<std::vector<float> > log_y;
  float logModifier; // a term added to lin_x and lin_y to make these loggable if necessary
  
  std::vector<QColor> plotColors;
  std::vector<bool> selectedChannels;
  bool plotLog;
  // all mins and maxes are in linear scale. Convert if necessary. Don't do log if negative values present
  // or possibly convert to something like v[i][j] + min + 0.01*max before logging 
  float x_min, x_max, y_min, y_max;        // actual mins and maxes
  float xp_min, xp_max, yp_min, yp_max;    // plotted mins and maxes

  // plotting variables
  int vMargin, hMargin;
  int tick_spacing, tick_length;
  float xScale, yScale;
  int m_size;  // marker size

  QPainterPath selectPath;
  QPainterPath logSelectPath;
  bool selectingPath;
};

#endif

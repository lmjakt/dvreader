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

#include "pointDrawer.h"
#include "distanceMapper.h"    // for the dpoint .. bit messy but there you go..
#include "BackgroundDrawer.h"
#include "DensityPlot.h"
#include "ColorScale.h"
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <QPainter>
#include <QPalette>
#include <QPrinter>
#include <QSvgGenerator>
#include <QSizeF>
#include <QColor>
#include <QRegion>
#include <QPixmap>
#include <iostream>
#include <QPolygon>
#include <QFileDialog>
#include <QDir>
#include <QMenu>
#include <QMouseEvent>
#include <QRubberBand>
#include <QRectF>

using namespace std;

PointDrawer::PointDrawer(QWidget* parent, const char* name)
  : QWidget(parent, name)
{
  setCaption("Distances");
  setBackgroundMode(Qt::NoBackground);    // but really should have something to put it there.. hmm
  movingId = -1;
  regions.resize(0);
  zoomRect.setRect(-1, -1, 0, 0);
  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
  QPalette bandPalette;
  bandPalette.setBrush(QPalette::Foreground, QBrush(Qt::green));
  bandPalette.setBrush(QPalette::Base, QBrush(Qt::red));
  rubberBand->setPalette(bandPalette);
  frameCounter = 0;
  point_plot_type = STRESS;
  annotation_field = "";
  diameter = 16;             // diameter for circles representing.. things.. 
  pos.setMargin(32);
  margin = 32;               // number of pixels around the edge that we don't want to draw..
  drawScale = 1.0;
  coord_sum_max = 0;
  coord_radius_factor = 0;
  draw_forces = true;
  draw_ids = true;
  labelPen = QPen(QColor(50, 50, 50), 0);
  menu = new QMenu(this);
  menu->addAction("Compare Cell Types", this, SLOT(compareCellTypes()) );
  menu->addAction("Set Coordinates", this, SLOT(setcoords()) );
  menu->addAction("Record on/off", this, SLOT(toggleRecording()) );

  bg_data = 0;
  bg_drawer = 0;
  
  defaultColors.push_back(QColor("blue"));
  defaultColors.push_back(QColor("green"));
  defaultColors.push_back(QColor("cyan"));
  defaultColors.push_back(QColor("red"));
  defaultColors.push_back(QColor("magenta"));
  defaultColors.push_back(QColor("yellow"));
  defaultColors.push_back(QColor("pink"));
  defaultColors.push_back(QColor("brown"));
  defaultColors.push_back(QColor("grey"));
}

PointDrawer::~PointDrawer(){
  cout << "destroying point drawer" << endl;
  delete []bg_data;
  delete bg_drawer;
}

void PointDrawer::emptyData(){
  points.resize(0);
}

void PointDrawer::setPointPlotType(PointPlotType ppt)
{
  point_plot_type = ppt;
  determine_coordinate_scale();
  update();
}

void PointDrawer::drawForces(bool b)
{
  draw_forces = b;
  update();
}

void PointDrawer::drawIds(bool b)
{
  draw_ids = b;
  update();
}

void PointDrawer::setPointDiameter(unsigned int d)
{
  diameter = d;
  margin = 2 * d;
  pos.setMargin(2 * d);
  determine_coordinate_scale();
  update();
}

void PointDrawer::setPlotScale(float s)
{
  drawScale = s;
  update();
}

void PointDrawer::setAnnotation(Annotation annot)
{
  if(annot.n_size() == points.size())
    annotation = annot;
}

void PointDrawer::plotAnnotationField(QString field)
{
  if(annotation.has_column(field)){
    point_plot_type = ANNOT;
    annotation_field = field;
    update();
  }
}

// set w & h to 0 to suppress drawing of the scale
void PointDrawer::drawAnnotationScale(int x, int y, int w, int h)
{
  annotation_scale.setRect(x, y, w, h);
  update();
}

void PointDrawer::setPointFilter(QString filter_field, std::set<float> filter_values, bool filter_inverse)
{
  annotation_filter_field = filter_field;
  annotation_filter_values = filter_values;
  annotation_filter_inverse = filter_inverse;
  update();
}

// w and h in points.
void PointDrawer::postscript(QString fname, float w, float h)
{
  QPrinter printer;
  printer.setPaperSize(QSizeF(w, h), QPrinter::Point);
  //printer.setResolution(600);  // this has weird effects
  printer.setOutputFormat(QPrinter::PostScriptFormat);
  printer.setOutputFileName(fname);  // can override to pdf if not .ps
  pos.setDims((int)w, (int)h);
  QPainter p(&printer);
  drawPicture(p);
}

void PointDrawer::svg(QString fname, int w, int h)
{
  QSvgGenerator gen;
  gen.setFileName(fname);
  gen.setSize(QSize(w, h));
  gen.setViewBox(QRect(0, 0, w, h));
  gen.setTitle("SOD");
  QPainter p;
  p.begin(&gen);
  drawPicture(p);
  p.end();
}

void PointDrawer::set_simple_gaussian_background(std::vector<unsigned int> dims,
						 unsigned char* color_matrix, float var)
{
  if(!bg_drawer)
    bg_drawer = new BackgroundDrawer(points, &pos);
  unsigned char* nbg = bg_drawer->simple_gaussian(dims, color_matrix, var);
  if(!nbg){
    cerr << "PointDrawer::set_simple_gaussian_background: failed to obtain background " << endl;
    return;
  }
  if(bg_data)
    delete []bg_data;
  bg_data = nbg;
  bg_image = QImage(bg_data, pos.w(), pos.h(), QImage::Format_ARGB32);
  update();
}

void PointDrawer::set_density_background(float rad_multiplier)
{
  int radius = abs((int)(rad_multiplier * (float)diameter / 2.0));
  if(!radius)
    return;
  std::vector<float> x;
  std::vector<float> y;
  x.reserve(points.size());
  y.reserve(points.size());
  for(uint i=0; i < points.size(); ++i){
    if(filterPoint(i))
      continue;
    x.push_back( points[i]->coordinates[0] );
    y.push_back( points[i]->coordinates[1] );
  }
  DensityPlot dplot(x, y, &pos);
  unsigned short max_value;
  
  unsigned short* density = dplot.densityPlot(radius, &max_value);
  //  unsigned short* density = dplot.densityPlot(pos.w(), pos.h(), diameter, &max_value);
  if(!density){
    cerr << "PointDrawer::set_density_background obtained a null map" << endl;
    return;
  }
  // convert to image somehow. Would like to use same HSV mapping as for
  // points, this will be slower, but since the units are quantised we don't
  // really need a map.

  ColorScale c_scale;
  unsigned char* cmap = c_scale.arrayedColorIndexUS(max_value);
  if(!cmap){
    delete []density;
    cerr << "PointDrawer::set_density_background obtained a null cmap" << endl;
    return;
  }
  // and now for the magic, stuff.
  unsigned char* new_bg = new unsigned char[ 4 * width() * height() ];
  for(unsigned int i=0; i < (width() * height()); ++i){
    unsigned int off_set = 4 * density[i];
    new_bg[i * 4] = cmap[ off_set + 3]; // blue
    new_bg[i * 4 + 1] = cmap[ off_set + 2]; // green
    new_bg[i * 4 + 2] = cmap[ off_set + 1]; // red
    new_bg[i * 4 + 3] = cmap[ off_set]; // alpha
  //  *(int*)(new_bg + i * 4) = *(int*)(cmap + 4 * density[i]);
  }
  delete []cmap;
  bg_image = QImage(new_bg, width(), height(), QImage::Format_ARGB32);
  if(bg_data)
    delete []bg_data;
  bg_data = new_bg;
  update();
}

void PointDrawer::setGrid(std::vector<dpoint*> grid)
{
  gridPoints = grid;
  update();
}

void PointDrawer::setData(vector<dpoint*> p){
  points = p;
  regions.resize(points.size());
  itsA = true;
  float minX, maxX, minY, maxY;
  // and work out max, min, etc..
  if(!points.size()){
    pos.setRanges(0, 0, 0, 0);
    //    maxX = minX = maxY = minY = 0;
    return;
  }
  if(points[0]->dimNo < 2){
    cerr << "we will be crashing soon. That really rather sucks a lot, but what can one do.. " << endl;
    return;
  }
  minX = maxX = points[0]->coordinates[0];
  minY = maxY = points[0]->coordinates[1];
  maxStress = points[0]->stress;
  for(uint i=0; i < points.size(); i++){
    if(points[i]->coordinates[0] > maxX){ maxX = points[i]->coordinates[0]; }
    if(points[i]->coordinates[0] < minX){ minX = points[i]->coordinates[0]; }
    if(points[i]->coordinates[1] > maxY){ maxY = points[i]->coordinates[1]; }
    if(points[i]->coordinates[1] < minY){ minY = points[i]->coordinates[1]; }
    if(points[i]->stress > maxStress){ maxStress = points[i]->stress; }
  }
  point_ranges.setCoords(minX, minY, maxX, maxY);
  pos.setRanges(minX, maxX, minY, maxY);
  // and call update.. !!! hooo hoo yeah....
  update();
}

void PointDrawer::paintEvent(QPaintEvent* e){
  pos.setDims(width(), height());
  // rather than try to work out areas and related stuff.. -just 
  // draw the whole thing to a pixmap and then bitBlt it.. shouldn't be 
  // too much of a problem.. 
  QPixmap pix(width(), height());
  pix.fill(QColor(0, 0, 0));     // black backround man it's good stuff.. 
  

  QPainter p(&pix);              // should be ok.. 
  drawPicture(p);

  // ando bitblt.. 
  bitBlt(this, 0, 0, &pix, 0, 0);
  
  if(frameCounter){
    QPainter p(this);
    p.setPen(QPen(QColor(255, 0, 0), 1));
    p.drawText(0, 0, width(), height(), Qt::AlignRight|Qt::AlignBottom, "Rec");
    QString fname;
    fname.sprintf("%s/img_%04d.jpg", dirName.latin1(), frameCounter);
    ++frameCounter;
    pix.save(fname, "JPEG", 100);
  }
}

void PointDrawer::drawPicture(QPainter& p)
{
  float stressMultiplier = 0;
  QString numString;  
  float forceMultiplier = 0.5;       // forces are too large, makes the picture too messy.. 
  p.scale(drawScale, drawScale);
  if(bg_image.width()){
    std::cout << "Calling drawImage " << bg_image.width() << "x" << bg_image.height() << std::endl;
    p.drawImage( QRect(0, 0, width(), height()), bg_image,
		 QRect(0, 0, bg_image.width(), bg_image.height()));
  }
  
  if(maxStress > 0){
    stressMultiplier = 254.0 / maxStress;     // not so good as we don't see any reduction in the max stress. 
  }
  // lets draw connections.. make optional later on..
  for(unsigned int i=0; i < points.size(); ++i)
    drawConnections(p, points[i]);
  // draw the forces first so that they are in the background
  if(draw_forces && points.size()){
    QPen attraction(QColor(255, 255, 0), 1);   // yellow
    QPen repulse(QColor(145, 152, 226), 1);    // light blue.. 
    // and let's go through the points and draw them as we see fit.. need some
    for(uint i=0; i < points.size(); i++){
      if(points[i]->dimNo < 2){
	cerr << "??? bugger me backwards, but the coordinates size is less than two for i : " << i << endl;
	continue;
      }
      int x = pos.x(points[i]->coordinates[0]);
      int y = pos.y(points[i]->coordinates[1]);
      for(unsigned int j=0; j < points[i]->componentNo; j++){
	float fx = points[i]->coordinates[0] + forceMultiplier * points[i]->components[j]->forces[0];
	float fy = points[i]->coordinates[1] + forceMultiplier * points[i]->components[j]->forces[1];
	int x2 = pos.x(fx);
	int y2 = pos.y(fy);
	if(points[i]->components[j]->attractive){
	  p.setPen(attraction);
	}else{
	  p.setPen(repulse);
	}
	p.drawLine(x, y, x2, y2);   // x2 and y2 are forces and are relative to the current position.. 
      }
    }
  }

  // if DOTS we need to prepare an offsets..
  std::vector<QPoint> dot_offsets;
  if(point_plot_type == DOTS)
    dot_offsets = makeDiskOffsets();
  // do point by point instead.
  QString label;
  for(uint i=0; i < points.size(); ++i){
    if(points[i]->dimNo < 2)
      continue;
    if(filterPoint(i))
      continue;
    int x = pos.x(points[i]->coordinates[0]);
    int y = pos.y(points[i]->coordinates[1]);
    label.setNum(points[i]->index);
    regions[i].setRect(x-diameter/2, y-diameter/2, diameter, diameter);
    if(point_plot_type == LEVELS_PIE){
      drawPie(p, points[i], x, y, label);
      continue;
    }
    // for the following the point diameter is constant so we can say
    x = x - (diameter / 2);
    y = y - (diameter / 2);
    if(point_plot_type == DOTS){
      drawDots(p, points[i], x, y, dot_offsets);
      continue;
    }
    QColor p_color(75, 75, 75);
    if(point_plot_type == STRESS){
      int r = (int)(points[i]->stress * stressMultiplier);
      p_color = QColor( r, 255-r, 0 );
    }
    if(point_plot_type == ANNOT)
      p_color = annotation.node_color(i, annotation_field);
    
    drawPoint(p, points[i], x, y, p_color);
  }
  if(point_plot_type == ANNOT && annotation_scale.width() && annotation_scale.height()){
    float min, max;
    unsigned int divs = 100;
    std::vector<QColor> colors = annotation.color_scale(annotation_field, divs, min, max); // 
    drawColorScale(p, colors, min, max, annotation_scale.left(), annotation_scale.top(),
		   annotation_scale.width(), annotation_scale.height() );
  }
  if(gridPoints.size()){
    p.setPen(labelPen);
    for(unsigned int i=0; i < gridPoints.size(); ++i)
      drawGridPoint(p, gridPoints[i]);
  }
  if(movingId != -1){
    p.setBrush(QColor(100, 100, 100));   // a gray shadow..
    p.setPen(Qt::NoPen);
    p.drawEllipse(movingRect);
  }
}

void PointDrawer::mousePressEvent(QMouseEvent* e){
  // if control key is pressed use it to scale the drawing region
  // by doing pos.setRange()
  Qt::KeyboardModifiers km = e->modifiers();
  if(km & Qt::ControlModifier){
    zoomOrigin = e->pos();
    zoomRect = QRect(zoomOrigin, QSize(0, 0));
    if(!rubberBand)
      rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
    rubberBand->setGeometry(zoomRect);
    rubberBand->show();
    return;
  }

  // if the click is inside a thingy..
  if(e->button() == Qt::RightButton){
    menu->popup(mapToGlobal(e->pos()));
    return;
  }
  for(uint i=0; i < regions.size(); i++){
    if(regions[i].contains(e->pos())){
      movingId = i;
      movingRect = regions[i];
      break;
    }
  }
  selectPoints.resize(0);    //hmm may be better to do else where... but .. 
  if(movingId == -1){
    selectPoints.push_back(e->pos());
  }

  lastX = e->x();
  lastY = e->y();
}

void PointDrawer::mouseMoveEvent(QMouseEvent* e){
  if(zoomRect.x() >= 0){
    zoomRect = QRect(zoomOrigin, e->pos()).normalized();
    rubberBand->setGeometry(zoomRect);
    rubberBand->show();
  }

  if(movingId != -1){
    movingRect.moveBy(e->x() - lastX, e->y() - lastY);
  }
  if(selectPoints.size()){
    selectPoints.push_back(e->pos());
  }
  lastX = e->x();
  lastY = e->y();
  if(movingId != -1){
    update();
  }
}

void PointDrawer::mouseReleaseEvent(QMouseEvent* e){
  // and emit something useful here so the mapper changes the coordinates and tries to update things..
  // do in a couple of different steps.. 
  if(zoomRect.x() >= 0){
    // do something useful..
    float min_x = pos.rx( zoomRect.left() );
    float max_x = pos.rx( zoomRect.right() );
    float min_y = pos.ry( zoomRect.top() ); // or other way around ??
    float max_y = pos.ry( zoomRect.bottom() );
    pos.setRanges(min_x, max_x, min_y, max_y);
    zoomRect.setRect(-1, -1, 0, 0);
    rubberBand->hide();
  }
  if(movingId != -1){
    //float xMult = ((float)(width() - 2 * margin))/(maxX - minX);
    //float yMult = ((float)(height() - 2 * margin))/(maxY - minY);
    float x = pos.rx( movingRect.x() + diameter / 2);
    float y = pos.ry( movingRect.y() + diameter / 2);
    //    float x = minX + ((float)(movingRect.x() + diameter/2 - margin))/xMult;
    //float y = minY + ((float)(movingRect.y() + diameter/2 - margin))/yMult;
    emit updatePosition(movingId, x, y);    // catch in the viewer, pass on to the mapper, and continue the mapping.. 
  }
  if(selectPoints.size()){
    checkSelected();
  }
  movingId = -1;
  update();
}

void PointDrawer::mouseDoubleClickEvent(QMouseEvent* e)
{
  // reset the zoom..
  cout << "mouse double click setting ranges to : "
       << point_ranges.left() << "->" << point_ranges.right() << "  : "
       << point_ranges.bottom() << "->" << point_ranges.top() << endl;
    
  pos.setRanges(point_ranges.left(), point_ranges.right(),
		point_ranges.top(), point_ranges.bottom());
  update();
}

void PointDrawer::checkSelected(){
  if(selectPoints.size() < 2){
    cerr << "Don't have any selected points.. ";
    return;
  }
  ///// Here's something really ugly,, create a QPointArray from the selectPoints.. -- why ? 
  /// because QPointArray
  ///// doesn't have any push, or reserve, or other useful things... so waste of time.. OK.
  QPolygon pts(selectPoints.size());
  for(uint i=0; i < selectPoints.size(); i++){
    pts.setPoint(i, selectPoints[i]);        // ugly, if I ever saw something...
  }
  // make a QRegion with these points.. and then check to see what's inside ..
  QRegion r(pts);
  set<uint>* a;
  set<uint>* b;
  cout << "itsA is " << itsA << endl;
  if(itsA){
    cout << "set A to be A " << endl;
    a = &selectedA;
    b = &selectedB;
  }else{
    cout << "set B to be a " << endl;
    b = &selectedA;
    a = &selectedB;
  }
  itsA = !itsA;
  cout << "what's up?" << endl;
  // empty a..
  while(a->size()){ a->erase(a->begin()); }
  for(uint i=0; i < points.size(); i++){
    int x = pos.x(points[i]->coordinates[0]);
    int y = pos.y(points[i]->coordinates[1]);
    if(r.contains(QPoint(x, y))){
      cout << "Point with index : " << points[i]->index << " is contained by the region " << endl;
      // stick everything into a..
      b->erase(i);          // don't check, just call erase, no harm if nothing there.. 
      a->insert(i);         // use the index from the vector rather than the internal index, it's easier..
    }
  }
  cout << "and then what ? " << endl;
  // and return ..
}

void PointDrawer::drawPoint(QPainter& p, dpoint* point, int x, int y, QColor color)
{
  p.save();
  p.setPen(Qt::NoPen);
  p.setBrush(color);
  p.drawEllipse(x, y, diameter, diameter);
  if(draw_ids){
    int extra = 20;
    p.setPen(labelPen);
    QString numString;
    numString.setNum(point->index);
    p.drawText( x-extra, y, diameter+extra*2, diameter, Qt::AlignCenter, numString);
  }
  p.restore();
}

void PointDrawer::drawPie(QPainter& p, dpoint* point, int x, int y, QString label)
{
  p.save();

  int radius = diameter / 2;
  int tbs = 50;
  p.setPen(Qt::NoPen);
  // if not position vector in point then just draw a red circle
  if(point->position.size() < 2){
    p.setBrush(QColor("red"));
    p.drawEllipse(x-radius, y-radius, diameter, diameter);
    p.setPen(labelPen);
    if(draw_ids)
      p.drawText(x-tbs, y-tbs, tbs*2, tbs*2, Qt::AlignCenter, label);
    p.restore();
    return;
  }
  float coord_sum = 0;
  for(unsigned int i=0; i < point->position.size(); ++i)
    coord_sum += point->position[i];
  coord_sum = coord_sum == 0 ? 1 : coord_sum;
  
  if(coord_radius_factor)
    radius = (int)( sqrt(coord_sum) / coord_radius_factor );
  int d = radius * 2;
  int full_circle = 360 * 16; // defined by Qt.
  int start_angle = 0;
  int span_angle = start_angle;
  QRect rect(x - radius, y-radius, d, d);
  for(unsigned int i=0; i < point->position.size(); ++i){
    p.setBrush(defaultColors[ i % defaultColors.size() ]);
    span_angle = (full_circle * point->position[i]) / coord_sum; 
    p.drawPie(rect, start_angle, span_angle);
    start_angle += span_angle;
  }
  p.setPen(labelPen);
  if(draw_ids)
    p.drawText(x-tbs, y-tbs, tbs*2, tbs*2, Qt::AlignCenter, label);
  p.restore();
}

// make point offsets suitable for plotting..
std::vector<QPoint> PointDrawer::makeDiskOffsets()
{
  std::vector<QPoint> cpoints;
  double min_distance = 4; // minimum distance between points.
  double r = (double)diameter / 2.0;
  for(double ri=min_distance; ri < r; ri += min_distance){
    double angle_increment = 2 * asin(min_distance/(2*ri));
    for(double angle = -M_PI; angle < M_PI; angle += angle_increment)
      cpoints.push_back( QPoint(round(ri * cos(angle)), round(ri * sin(angle))) );
  }
  return(cpoints);
}

void PointDrawer::drawDots(QPainter& p, dpoint* point, int x, int y, std::vector<QPoint>& offsets)
{
  p.save();

  // work out how many dots to draw
  float coord_sum = 0;
  for(uint i=0; i < point->position.size(); ++i)
    coord_sum += point->position[i];
  uint draw_length = offsets.size();
  if(coord_sum_max)
    draw_length = (int)( (float)draw_length * coord_sum / coord_sum_max );
  draw_length = draw_length > offsets.size() ? offsets.size() : draw_length;
  draw_length = !draw_length ? 1 : draw_length;
  // this is a pretty ugly way of getting proportional drawing..
  std::vector<QColor> colors;
  unsigned int col_length = 100;
  for(unsigned int i=0; i < point->position.size(); ++i){
    cout << "\t" << point->position[i];
    unsigned int c = uint(float(col_length) * point->position[i] / coord_sum);
    cout << " : " << c;
    for(uint j=0; j < c; ++j)
      colors.push_back(defaultColors[ i % defaultColors.size() ]);
  }
  cout << endl;
  if(!colors.size())
    colors.push_back(QColor(255, 255, 255));

  QPoint cp(x, y);  
  std::cout << "drawDots " << coord_sum << " : " << coord_sum_max << "  colors.size(): " << colors.size() << endl;
  for(uint i=0; i < draw_length; ++i){
    p.setPen(QPen(colors[ rand() % colors.size() ], 2)); // we should call srand somewhere.. but.. 
    p.drawPoint(cp + offsets[i]);
  }
  p.restore();
}

void PointDrawer::drawConnections(QPainter& p, dpoint* point)
{
  //cout << "DrawConnections size " << point->neighbor_indices.size() << endl;
  if(!point->neighbor_indices.size())
    return;
  p.save();
  p.setPen(QPen(QColor(40, 40, 175), 1));
  int x1, x2, y1, y2;
  x2 = x1 = pos.x(point->coordinates[0]);
  y2 = y1 = pos.y(point->coordinates[1]);
  for(set<unsigned int>::iterator it = point->neighbor_indices.begin();
      it != point->neighbor_indices.end(); ++it){
    if((*it) < points.size()){
      x2 = pos.x(points[*it]->coordinates[0]);
      y2 = pos.y(points[*it]->coordinates[1]);
      p.drawLine(x1, y1, x2, y2);
    }
  }
  p.restore();
}

void PointDrawer::drawGridPoint(QPainter& p, dpoint* gpoint)
{
  if(gpoint->dimNo < 2)
    return;
  int x1 = pos.x(gpoint->coordinates[0]);
  int y1 = pos.y(gpoint->coordinates[1]);
  std::cout << "from " << x1 << "," << y1 << ":";
  for(unsigned int j=0; j < gpoint->position.size(); ++j)
    cout << "\t" << (int)gpoint->position[j];
  cout << endl;
  for(unsigned int i=0; i < gpoint->neighbors.size(); ++i){
    if(!gpoint->neighbors[i])
      continue;
    if(gpoint->neighbors[i]->dimNo < 2)
      continue;
    p.setPen(QPen(defaultColors[i % defaultColors.size()], 0));
    cout << i << " color " << defaultColors[i % defaultColors.size()].name().toAscii().constData() << endl;
    int x2 = pos.x(gpoint->neighbors[i]->coordinates[0]);
    int y2 = pos.y(gpoint->neighbors[i]->coordinates[1]);
    cout << "\t" << i << " -->  " << x2 << "," << y2 << ":";
    for(unsigned int j=0; j < gpoint->neighbors[i]->position.size(); ++j)
      cout << "\t" << (int)gpoint->neighbors[i]->position[j];
    cout << endl;
    p.drawLine(x1, y1, x2, y2);
  }
}

// width and height are for the complete scale
// hence increments will be drawn as boxes at width / colors.size()
void PointDrawer::drawColorScale(QPainter& p, std::vector<QColor> colors, float min, float max,
				 int x, int y, int w, int h)
{
  if(w <= 0 || h <= 0)
    return;
  float step_size = (float)w / (float)colors.size();
  float b_width = step_size > 1 ? step_size : 2; // minimum size

  QRectF box((float)x, (float)y, b_width, (float)h);
  p.setPen(Qt::NoPen);
  for(unsigned int i=0; i < colors.size(); ++i){
    p.setBrush( colors[i] );
    p.drawRect(box);
    box.translate(step_size, 0);
  }
}

void PointDrawer::determine_coordinate_scale()
{
  coord_sum_max = 0;
  for(unsigned int i=0; i < points.size(); ++i){
    float coord_sum = 0;
    for(unsigned int j=0; j < points[i]->position.size(); ++j)
      coord_sum += points[i]->position[j];
    if(coord_sum > coord_sum_max) coord_sum_max = coord_sum;
  }
  // determine a scale such that 
  // coord_radius_factor * sqrt(coord_sum_max) = (diameter / 2)
  if(!coord_sum_max){
    coord_radius_factor = 0;
    return;
  }
  float rf = float(diameter)/2.0;
  coord_radius_factor = coord_sum_max / (rf * rf);
  coord_radius_factor = sqrt(coord_radius_factor);
}

// to filter is to remove something. Hence if the criteria fit
// we return true. (i.e. don't draw it).
// unless annotation_filter_inverse is false.
bool PointDrawer::filterPoint(unsigned int i)
{
  if(!annotation_filter_values.size() ||  annotation.n_size() != points.size() )
    return(false); // i.e. don't filter it.
  bool f = annotation.filter(i, annotation_filter_field, annotation_filter_values);
  if(annotation_filter_inverse)
    f = !f;
  return(f);
}

void PointDrawer::compareCellTypes(){
  if(!selectedA.size() || !selectedB.size()){
    cerr << "Cant compare nothing against something, now can I, you fool !!" << endl;
    return;
  }
  cout << "Should be emitting something useful for the parents to take care of.. " << endl;
  vector<int> a;
  vector<int> b;
  set<uint>::iterator it;
  for(it = selectedA.begin(); it != selectedA.end(); it++){
    // ugly,,
    if(*it < points.size()){
      a.push_back(points[*it]->index);
    }
  }
  for(it = selectedB.begin(); it != selectedB.end(); it++){
    if(*it < points.size()){
      b.push_back(points[*it]->index);
    }
  }
  emit compareCells(a, b);     // the order doesn't matter so much.. 
		  
}

void PointDrawer::setcoords(){
  cout << "point drawer emitting setCoordinates : " << endl;
  emit setCoordinates();  // and let our owner -- have a look at what the coordinates really are.. hmm .
}

void PointDrawer::toggleRecording(){
    if(frameCounter){
	frameCounter = 0;
	update();
	return;
    }
    // otherwise get a directory name and set the frameCounter to 1.
    QDir dir;
    bool dirMade = false;
    while(!dirMade){
	dirName = QFileDialog::getSaveFileName();
	cout << "dirName is " << dirName.toAscii().data() << endl;
	if(dirName.isNull()){
	    return;
	}
	dirMade = dir.mkdir(dirName);
    }
    frameCounter=1;
    update();
}

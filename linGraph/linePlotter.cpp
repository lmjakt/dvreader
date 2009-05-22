#include "linePlotter.h"
#include <iostream>
#include <QPalette>
#include <QPainter>
#include <QPen>
#include <QMouseEvent>
#include <QLineF>

using namespace std;


LinePlotter::LinePlotter(QWidget* parent)
    : QWidget(parent)
{
    // we could set background colors here but maybe we can find out something about the
    // current palette
    QPalette palette;
    QColor background = palette.color(QPalette::Active, QPalette::Background);
    QColor foreground = palette.color(QPalette::Active, QPalette::Foreground);
    QColor base = palette.color(QPalette::Active, QPalette::Base);
    QColor text = palette.color(QPalette::Active, QPalette::Text);
    QColor button = palette.color(QPalette::Active, QPalette::Button);
    QColor buttonText = palette.color(QPalette::Active, QPalette::ButtonText);
    cout << "Active background " << background.red() << "," << background.green() << "," << background.blue() << "\n"
	 << "Active foregrond  " << foreground.red() << "," << foreground.green() << "," << foreground.blue() << "\n"
	 << "Active base       " << base.red() << "," << base.green() << "," << base.blue() << "\n"
	 << "Active text       " << text.red() << "," << text.green() << "," << text.blue() << "\n"
	 << "Active button     " << button.red() << "," << button.green() << "," << button.blue() << "\n"
	 << "Button text       " << buttonText.red() << "," << buttonText.green() << "," << buttonText.blue() << endl;

    vMargin = 10;
    hMargin = 10;
    tick_spacing = 30;
    tick_length = 5;
    xScale = 1.0;
    yScale = 1.0;

    setFocusPolicy(Qt::StrongFocus);
    
}

LinePlotter::~LinePlotter(){
    // do something ?
}

void LinePlotter::setData(vector< vector<float> >& v, vector<QColor>& c){
    values = v;
    colors = c;
    
    if(!values.size() || !values[0].size() || !colors.size()){
	cerr << "LinePlotter::setData No values in first array. Giving up. Undefined behaviour perhaps" << endl;
	return;
    }
    min = max = values[0][0];
    maxLength = values[0].size();
    for(uint i=0; i < values.size(); ++i){
	maxLength = maxLength < values[i].size() ? values[i].size() : maxLength;
	for(uint j=0; j < values[i].size(); ++j){
	    max = max < values[i][j] ? values[i][j] : max;
	    min = min > values[i][j] ? values[i][j] : min;
	}
    }
    // recycle colors if necessary.. 
    if(colors.size() < values.size()){
	uint i = 0;
	while(colors.size() < values.size()){
	    colors.push_back( colors[ i % colors.size() ] );
	    ++i;
	}
    }
    update();
}

// void LinePlotter::paintEvent(QPaintEvent* e){
//     e = e;  // to avoid annoying warning
//     int w = width() - hMargin * 2;
//     int h = height() - hMargin * 2;
//     int hb = height() - hMargin;
//     float range = max - min;

//     QPainter p(this);
//     p.drawLine(hMargin, height()-vMargin, width()-hMargin, height()-vMargin);
//     p.drawLine(hMargin, height()-vMargin, hMargin, vMargin);
//     p.scale(xScale, yScale);
//     // vertical ticks
//     for(int yp = height()-vMargin; yp >= vMargin; yp -= tick_spacing)
// 	p.drawLine(hMargin-tick_length, yp, hMargin, yp);
//     for(int xp = hMargin; xp <= width()-hMargin; xp += tick_spacing)
// 	p.drawLine(xp, hb + tick_length, xp, hb);
    
//     // Draw the values.
//     if(!range){
// 	cerr << "LinePlotter::paintEvent range is 0" << endl;
// 	return;
//     }
//     cout << "LinePloter::paintEvent values.size : " << values.size() << endl;
//     for(uint i=0; i < values.size(); ++i){
// 	p.setPen(QPen(colors[i], 1));
// 	int x, y;
// 	int px = hMargin;
// 	int py = (int)( hb - ((values[i][0] - min) * (float)h) / range ); 
// 	for(uint j=1; j < values[i].size(); ++j){
// 	    x = hMargin + (w * j) / maxLength;
// 	    y = (int)( hb - ((values[i][j] - min) * (float)h) / range ); 
// 	    p.drawLine(px, py, x, y);
// 	    px = x;
// 	    py = y;
// 	    //    if(i > 0)
// 	    //cout << "\t" << j << "\t" << values[i-1][j] << " : " << values[i][j] << endl;
// 	}
//     }

// }

void LinePlotter::paintEvent(QPaintEvent* e){
    e = e;  // to avoid annoying warning
    float w = (float)width() - hMargin * 2;
    float h = (float)height() - vMargin * 2;
    //int hb = height() - vMargin;
    float range = max - min;

    QPainter p(this);

    p.translate(0, height());
    p.scale(1.0, -1.0);
    p.translate(hMargin, vMargin);
    for(int xp = 0; xp <= w; xp += tick_spacing)
	p.drawLine(xp, -tick_length, xp, 0);
    p.drawLine(0, 0, w, 0);
    p.drawLine(0, 0, 0, h);

    // Note that scaling the painter doesn't really work very well.
    // as it scales from 0.1 as 0 meaning that values below 1.0 don't
    // actually scale at all.
    //p.scale(xScale, yScale);
    // vertical ticks
    for(int yp = h; yp >= 0; yp -= tick_spacing)
	p.drawLine(-tick_length, yp, 0, yp);
    
    // Draw the values.
    if(!range){
	cerr << "LinePlotter::paintEvent range is 0" << endl;
	return;
    }
    cout << "LinePloter::paintEvent values.size : " << values.size() << endl;
    for(uint i=0; i < values.size(); ++i){
	p.setPen(QPen(colors[i], 0));
	qreal x, y;
	qreal px = 0;
	qreal py =  yScale * (values[i][0] - min) * h / range ; 
	for(uint j=1; j < values[i].size(); ++j){
	  x = (w * (float)j) / maxLength;
	  y = yScale * (values[i][j] - min) * h / range ; 
	  QLine fline(px, py, x, y);
	  p.drawLine(fline);
	  //	    p.drawLine(px, py, x, y);
	  px = x;
	  py = y;
	  //    if(i > 0)
	  //cout << "\t" << j << "\t" << values[i-1][j] << " : " << values[i][j] << endl;
	}
    }

}


void LinePlotter::mouseDoubleClickEvent(QMouseEvent* e){
    e = e;
    emit doubleClicked();
}

void LinePlotter::mouseMoveEvent(QMouseEvent* e){
//    int x = e->x();
//    int y = e->y();
    emitMousePos(e->x(), e->y());
//     if(x < hMargin || x > width() - hMargin || y < vMargin || y > height() - vMargin)
// 	return;
//     int xp = ((x - hMargin) * maxLength) / (width() - 2 * hMargin);
//     float yp =  min + (max - min) *  (float)((height() - vMargin) - y) / (float)(height() - vMargin*2);
//     emit mousePos(xp, yp);
}

void LinePlotter::mousePressEvent(QMouseEvent* e){
    emitMousePos(e->x(), e->y());
}

void LinePlotter::keyPressEvent(QKeyEvent* e){
  cout << "keyPressEvent .." << endl;
  switch(e->key()){
  case Qt::Key_Up :
    yScale *= 1.5;
    break;
  case Qt::Key_Down :
    yScale /= 1.5;
    break;
  case Qt::Key_Space :
    yScale = 1.0;
    break;
  default:
    e->ignore();
  }
  cout << "LinePlotter keyPressEvent yScale is now : " << yScale << endl;
  update();
}

void LinePlotter::emitMousePos(int x, int y){
    if(x < hMargin || x > width() - hMargin || y < vMargin || y > height() - vMargin)
	return;
    int xp = ((x - hMargin) * maxLength) / (width() - 2 * hMargin);
    float yp =  min + (max - min) * ( (float)((height() - vMargin) - y) / (float)(height() - vMargin*2) ) / yScale;
    emit mousePos(xp, yp);
}

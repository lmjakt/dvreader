#include "blobMapperWidget.h"
#include <iostream>
#include <QLayout>
#include <QPalette>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QColorDialog>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <sstream>

using namespace std;

BlobMapperWidget::BlobMapperWidget(BlobMapper* bmapper, fluorInfo& fInfo, string fname, QColor c, QWidget* parent)
    : QWidget(parent)
{
    mapper = bmapper;
    fluor = fInfo;
    fileName = fname;

    // make all static tool buttons with icons before calling makeIcons
    eatNeighborButton = new QToolButton(this);
    eatNeighborButton->setToolTip("Eat Neighbours");
    connect(eatNeighborButton, SIGNAL(clicked()), this, SLOT(eatNeighbors()));

    drawDisButton = new QToolButton(this);
    drawDisButton->setToolTip("Plot distribution");
    drawDisButton->setCheckable(true);
    drawDisButton->setChecked(true);
    connect(drawDisButton, SIGNAL(clicked()), this, SIGNAL(includeDistChanged()) );

    makeIcons();  // this will set the icon for the eatNeighbor button

    
    currentColor = c;
    //    currentColor = QColor(255, 0, 255);
    QPalette palette(currentColor);
    colorButton = new QToolButton(this);
    colorButton->setPalette(palette);
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setColor()) );

    currentRep = 1; // 0, 1, 2 should refer to RepIcon::NO_REP, RepIcon::OUTLINE, RepIcon::FILLED. This is set in the makeIcons function.
    drawTypeButton = new QToolButton(this);
    drawTypeButton->setIcon(icons[currentRep].icon);
    connect(drawTypeButton, SIGNAL(clicked()), this, SLOT(changeRep()));

    QString labString;
    labString.setNum(fluor.excitation);
    QLabel* exciteLabel = new QLabel(labString, this);
    labString.setNum(fluor.emission);
    QLabel* emitLabel = new QLabel(labString, this);
    labString.setNum(mapper->minimum());
    QLabel* minLabel = new QLabel(labString, this);
    
    QPushButton* delButton = new QPushButton("del", this);
    delButton->setMaximumWidth(40);
    connect(delButton, SIGNAL(clicked()), this, SIGNAL(deleteMe()));
    // not connected yet..
    
    QHBoxLayout* hbox = new QHBoxLayout(this);
    hbox->setMargin(2);
    hbox->setSpacing(1);
    hbox->addWidget(delButton);
    hbox->addWidget(eatNeighborButton);
    //hbox->addWidget(exportButton);
    hbox->addStretch();
    hbox->addWidget(exciteLabel);
    hbox->addSpacing(3);
    hbox->addWidget(emitLabel);
    hbox->addWidget(minLabel);
    hbox->addStretch();
    hbox->addWidget(colorButton);
    hbox->addWidget(drawTypeButton);
    hbox->addWidget(drawDisButton);
}

BlobMapperWidget::~BlobMapperWidget(){
    delete mapper;
}

set<blob*>& BlobMapperWidget::blobs(){
    return(mapper->gBlobs());
}

BlobMapper* BlobMapperWidget::blobMapper(){
    return(mapper);
}

RepIcon::BlobRepresentation BlobMapperWidget::blobRep(){
    return( icons[currentRep].rep);
}

void BlobMapperWidget::color(float& r, float& g, float& b){
    r = (float)currentColor.red() / 255.0;
    g = (float)currentColor.green() / 255.0;
    b = (float)currentColor.blue() / 255.0;
}

QColor BlobMapperWidget::color(){
    return(currentColor);
}

bool BlobMapperWidget::plotDistribution(){
  return(drawDisButton->isChecked());
}

map<BlobMapperWidget::Param, pl_limits> BlobMapperWidget::pLimits(){
  cout << "BlobMapperWidget returning pLimits (size of) : " << plotLimits.size() << endl;
  return(plotLimits);
}

void BlobMapperWidget::setPlotLimits(Param p, float l, float r){
  cout << "BlobMapper Widget setting plot limits " << p << "  : " << l << " : " << r << endl;
  plotLimits[p] = pl_limits(l, r);
}

void BlobMapperWidget::clearPlotLimits(){
  plotLimits.clear();
}

void BlobMapperWidget::clearPlotLimits(Param p){
  plotLimits.erase(p);
}

bool BlobMapperWidget::filterBlob(blob* b){
  for(map<Param, pl_limits>::iterator it=plotLimits.begin(); it != plotLimits.end(); ++it){
    float v = getParameter(b, (*it).first);
    if(v == -1){
      cerr << "BlobMapperWidget::filterBlob unknown Parameter : " << (*it).first << endl;
      return(true);
    }
    if(v < (*it).second.left || v > (*it).second.right)
      return(false);
  }
  return(true);
}

float BlobMapperWidget::getParameter(blob* b, Param p){
  float v = -1;
  int c = 0;
  switch(p){
  case VOLUME:
    v = (float)b->points.size();
    break;
  case SUM:
    v = b->sum;
    break;
  case MEAN:
    v = b->sum / (float)b->points.size();
    break;
  case MAX:
    v = b->max;
    break;
  case MIN:
    v = b->min;
    break;
  case EXTENT:
    v = (1 + b->max_x - b->min_x) * (1 + b->max_y - b->min_y) * (1 + b->max_z - b->min_z);
    break;
  case SURFACE:
      v = 0;
      for(uint i=0; i < b->surface.size(); ++i){
	  if(b->surface[i])
	      ++v;
      }
      break;
  case BACKGROUND:
      v = 0;
      c = 0;
      for(uint i=0; i < b->points.size(); ++i){
	  if(b->surface[i]){
	      v += mapper->g_value(b->points[i]);
	      ++c;
	  }
      }
      v = v / (float)c;
      break;
   default :
       v = -1;
    cerr << "BlobMapperWidget::getParameter unknown parameter " << p << "  returning -1 " << endl;
  }
  return(v);
}

void BlobMapperWidget::exportBlobs(){
    ostringstream fname;
    fname << fileName << "_" << fluor.excitation << "-" << fluor.emission << "_" << mapper->minimum() << ".blobs";
    cout << "Calling exportBlobs with file name : " << fname.str() << endl;
    mapper->exportBlobs(fname.str());
}

void BlobMapperWidget::setColor(){
    currentColor = QColorDialog::getColor(currentColor);
    QPalette palette(currentColor);
    //palette.setColor(QPalette::Button, currentColor);
    colorButton->setPalette(palette);
    emit newColor();
}

void BlobMapperWidget::changeRep(){
    currentRep = (++currentRep) % icons.size();
    drawTypeButton->setIcon(icons[currentRep].icon);
    cout << "Changing representation to " << currentRep << " : " << icons[currentRep].rep << endl;
    emit newRep();
}

void BlobMapperWidget::eatNeighbors(){
    mapper->eatNeighbors();
}

void BlobMapperWidget::makeIcons(){
    int w, h;
    w = h = 32;
    int m = 4;

    QPixmap pix(w, h);
    QPainter p(&pix);
    QColor bg(QColor(0, 0, 0));
    QColor fg(QColor(255, 255, 255));

    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(fg, 3));

    // first draw the representation for an empty one.
    pix.fill(bg);
    icons.push_back(RepIcon( QIcon(pix), RepIcon::NO_REP) );

    p.drawEllipse(m-1, m-1, w-2*m, h-2*m);
    icons.push_back(RepIcon( QIcon(pix), RepIcon::OUTLINE) );

    pix.fill(bg);
    p.setBrush(fg);
    p.drawEllipse( (w/2)-2, (h/2)-2, 4, 4);
    icons.push_back(RepIcon( QIcon(pix), RepIcon::PEAK) );
    
    p.drawEllipse(m-1, m-1, w-2*m, h-2*m);
    icons.push_back(RepIcon( QIcon(pix), RepIcon::FILLED) );

    // And draw an icon for eating neighbors
    pix.fill(bg);
    p.drawPie(m-1, m-1, w-2*m, w-2*m, 45*16, 270*16);
    p.drawPie(m, m-1, w-2*m, w-2*m, 15*16, -30*16);
    eatNeighborButton->setIcon(QIcon(pix));

    pix.fill(bg);
    QIcon disIcon;
    p.setBrush(Qt::NoBrush);
    QPainterPath path;
    path.moveTo(0, h);
    path.cubicTo(w/5, -h, w/2, h/0.9, w, h);
    p.drawPath(path);
    disIcon.addPixmap(pix);
    p.setBrush(fg);
    p.drawPath(path);
    disIcon.addPixmap(pix, QIcon::Normal, QIcon::On);
    drawDisButton->setIcon(disIcon);
    
}
   

#include "overlapEditor.h"
#include "overlapEditorWindow.h"
#include "../dataStructs.h"
#include "../opengl/glImage.h"
#include <QGridLayout>
//#include <QGraphicsView>
#include <QMatrix>
#include <QMouseEvent>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <iostream>
#include <set>

using namespace std;

OverlapEditorWindow::OverlapEditorWindow(QWidget* parent)
  : QWidget(parent)
{
  tex_width = 128;
  tex_height = 128;
  t_big = 8;
  t_small = 1;

  oEdit = new OverlapEditor(true, this);
  oEdit->setFocus();
  connect(oEdit, SIGNAL(r_delta_y(int)), this, SLOT(r_delta_y(int)) );
  connect(oEdit, SIGNAL(newFrameSelected(float, float)), 
	  this,  SLOT(frameSelected(float, float)) );
  connect(oEdit, SIGNAL(offSetChanged(QPoint)),
	  this, SLOT(offSetChanged(QPoint)) );
  //view = new QGraphicsView(oEdit);
  //viewScale = 1.0;
  
  //  vertImage = new GLImage(1, 1, 128, 1024, 1.0);
  leftImage = new GLImage(t_small, t_big, tex_width, tex_height, 1.0, this);
  rightImage = new GLImage(t_small, t_big, tex_width, tex_height, 1.0, this);
  topImage = new GLImage(t_big, t_small, tex_width, tex_height, 1.0, this);
  bottomImage = new GLImage(t_big, t_small, tex_width, tex_height, 1.0, this);

  // send key events to the overlapEditor
  connect(leftImage, SIGNAL(keyPressed(QKeyEvent*)), oEdit, SLOT(externalKey(QKeyEvent*)) );
  connect(rightImage, SIGNAL(keyPressed(QKeyEvent*)), oEdit, SLOT(externalKey(QKeyEvent*)) );
  connect(topImage, SIGNAL(keyPressed(QKeyEvent*)), oEdit, SLOT(externalKey(QKeyEvent*)) );
  connect(bottomImage, SIGNAL(keyPressed(QKeyEvent*)), oEdit, SLOT(externalKey(QKeyEvent*)) );

  borderImage = 0;

  QPushButton* adjustButton = new QPushButton("Adjust Position", this);
  connect(adjustButton, SIGNAL(clicked()), this, SLOT(requestAdjustment()));
  connect(oEdit, SIGNAL(requestAdjustment()), this, SLOT(requestAdjustment()) );

  QPushButton* updateButton = new QPushButton("Save Changes", this);
  connect(updateButton, SIGNAL(clicked()), this, SIGNAL(updateFileSetInfo()) );

  scaleBox = new QDoubleSpinBox(this);
  scaleBox->setRange(0.1, 10.0);
  scaleBox->setSingleStep(0.1);
  scaleBox->setValue(1.0);
  connect(scaleBox, SIGNAL(valueChanged(double)), this, SLOT(setScale(double)) );

  QGridLayout* grid = new QGridLayout(this);
  grid->addWidget(bottomImage, 2, 1);
  grid->addWidget(leftImage, 1, 0);
  grid->addWidget(rightImage, 1, 2);
  grid->addWidget(topImage, 0, 1);
  grid->addWidget(oEdit, 1, 1);

  controlGrid = new QGridLayout();
  grid->addLayout(controlGrid, 2, 0);
  controlGrid->addWidget(adjustButton, 0, 0);
  controlGrid->addWidget(scaleBox, 1, 0);
  controlGrid->addWidget(updateButton, 2, 0);

  // without the fixed size, things look crap
  // since I don't have a sizeHint for the ovlerlapEditor
  int imageThickness = 150;
  leftImage->setFixedWidth(imageThickness);
  rightImage->setFixedWidth(imageThickness);
  topImage->setFixedHeight(imageThickness);
  bottomImage->setFixedHeight(imageThickness);

}

void OverlapEditorWindow::setInfo(FileSetInfo* fsetInfo)
{
  oEdit->setInfo(fsetInfo, 1.0);
  float minR = 1.0;
  viewScale = minR;
}

void OverlapEditorWindow::setBorderImages(BorderInfo* binfo){
  if(borderImage)
    delete borderImage;
  borderImage = binfo;
  setChannelChecks( borderImage->channels() );
  oEdit->setOffset(borderImage->x(), borderImage->y(), QPoint(0, 0));
  binfo->setScale(scaleBox->value());
  clearImages();
  setOverlapImages();
}

void OverlapEditorWindow::r_delta_y(int dy){
  incrementScale(pow(2, ((float)dy / 10.0) ));
}

void OverlapEditorWindow::incrementScale(float ds){
  viewScale *= ds;

}

void OverlapEditorWindow::offSetChanged(QPoint p){
  cout << "New overlap obtaind " << p.x() << "," << p.y() << endl;
  if(!borderImage)
    return;
  borderImage->setOffset(p);
  clearImages();
  setOverlapImages();
}

void OverlapEditorWindow::frameSelected(float x, float y){
  // if(borderImages.count(x) && borderImages[x].count(y)){
  //   setBorderImages(borderImages[x][y]);
  //   return;
  // }
  if(borderImage && borderImage->x() == x && borderImage->y() == y)
    return;
  emit newFrameSelected(x, y);
}

void OverlapEditorWindow::requestAdjustment(){
  if(!borderImage)
    return;
  emit adjustFramePos(borderImage->x(), borderImage->y(), borderImage->offset());
}

void OverlapEditorWindow::setChannelChecks(std::set<int> channels){
  QString label;
  int pos = 0;
  for(set<int>::iterator it=channels.begin(); it != channels.end(); it++){
    if(!channelChecks.count((*it))){
      label.setNum(*it);
      QRadioButton* box = new QRadioButton(label, this);
      connect(box, SIGNAL(clicked()), this, SLOT(setOverlapImages()) );
      channelChecks.insert(make_pair(*it, box));
    }
    controlGrid->addWidget( channelChecks[*it], ++pos, 1);
  }
}

void OverlapEditorWindow::setScale(double s){
  if(borderImage){
    borderImage->setScale((float)s);
    setOverlapImages();
  }
}

void OverlapEditorWindow::clearImages(){
  float* blank = new float[ t_big * t_small * tex_width * tex_height * 3 ];
  memset(blank, 0, sizeof(float) * 3 * t_big * t_small * tex_width * tex_height);

  leftImage->setBigImage(blank, 0, 0, (t_small * tex_width), (t_big * tex_height) );
  rightImage->setBigImage(blank, 0, 0, (t_small * tex_width), (t_big * tex_height) );
  topImage->setBigImage(blank, 0, 0, (t_big * tex_width), (t_small * tex_height) );
  bottomImage->setBigImage(blank, 0, 0, (t_big * tex_width), (t_small * tex_height) );
  
  updateGL();
}

void OverlapEditorWindow::setOverlapImages(){
  if(!borderImage)
    return;
  
  int wave = 0;
  int i = 0;
  for(map<int, QRadioButton*>::iterator it=channelChecks.begin();
      it != channelChecks.end(); ++it){
    if((*it).second->isChecked())
      wave = i;
    ++i;
  }
  color_map t(1.0, 0.0, 0.0);
  color_map n(0.0, 1.0, 0.0);
  
  paintOverlap(leftImage, LEFT, wave, t, n);
  paintOverlap(rightImage, RIGHT, wave, t, n);
  paintOverlap(topImage, TOP, wave, t, n);
  paintOverlap(bottomImage, BOTTOM, wave, t, n);

}

void OverlapEditorWindow::paintOverlap(GLImage* image, POSITION pos, int wave, color_map& t, color_map& n){
  if(!borderImage)
    return;
  int w, h;
  float* data = borderImage->paint_overlap(pos, wave, w, h, t, n);
  image->setBigImage(data, 0, 0, w, h);
  image->updateGL();
  delete data;
}

void OverlapEditorWindow::updateGL(){
  leftImage->updateGL();
  rightImage->updateGL();
  topImage->updateGL();
  bottomImage->updateGL();

}

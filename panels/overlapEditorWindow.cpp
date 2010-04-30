#include "overlapEditor.h"
#include "overlapEditorWindow.h"
#include "../opengl/glImage.h"
#include <QGridLayout>
#include <QGraphicsView>
#include <QMatrix>
#include <QMouseEvent>
#include <iostream>


using namespace std;

OverlapEditorWindow::OverlapEditorWindow(QWidget* parent)
  : QWidget(parent)
{
  oEdit = new OverlapEditor(this);
  connect(oEdit, SIGNAL(r_delta_y(int)), this, SLOT(r_delta_y(int)) );
  view = new QGraphicsView(oEdit);
  viewScale = 1.0;
  
  vertImage = new GLImage(1, 1, 128, 1024, 1.0);
  //vertImage = new GLImage(1, 8, 128, 128, 1.0);
  vertImage->resize(128, 1024);
  vertImage->show();
  
  // lets try the set big image ..
  int test_w = 200;
  int test_h = 1024;
  float* testImage = new float[test_w * test_h * 3];
  for(int y=0; y < test_h; ++y){
    for(int x=0; x < test_w; ++x){
      int o = 3 * (y * test_w + x);
      testImage[o] = 1.0 - ((float)y / 1024.0);
      testImage[o+1] = (float)y / 1024.0;
      testImage[o+2] = float(y % 256) / 256;
    }
  }
  //  vertImage->setBigImage(testImage, 0, 0, test_w, test_h);
  vertImage->updateGL();
  delete(testImage);
  QGridLayout* grid = new QGridLayout(this);
  //grid->addWidget(vertImage, 0, 1);
  grid->addWidget(view, 1, 1);

}

void OverlapEditorWindow::setInfo(FileSetInfo* fsetInfo)
{
  oEdit->setInfo(fsetInfo);
  float minR = 
    ( float(view->width()) / oEdit->width() ) < ( float(view->height()) / oEdit->height() ) 
    ? 
    ( float(view->width()) / oEdit->width() ) : ( float(view->height()) / oEdit->height() );

  viewScale = minR;
  scaleView(viewScale);
  show();
}

void OverlapEditorWindow::r_delta_y(int dy){
  incrementScale(pow(2, ((float)dy / 10.0) ));
}

void OverlapEditorWindow::incrementScale(float ds){
  viewScale *= ds;
  scaleView(viewScale);
}

void OverlapEditorWindow::scaleView(float scale){
  view->resetMatrix();
  view->translate(0, view->height());
  view->scale(scale, -scale);
}

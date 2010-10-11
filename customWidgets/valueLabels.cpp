#include "valueLabels.h"
#include <QLabel>
#include <QString>
//#include <QHBoxLayout>
#include <QGridLayout>

using namespace std;

ValueLabels::ValueLabels(vector<QColor>& colors, QWidget* parent)
  : QWidget(parent)
{
  QPalette qp(palette());
  //QHBoxLayout* hbox = new QHBoxLayout(this);
  QGridLayout* grid = new QGridLayout(this);
  int maxColNo = 3;
  labels.resize(colors.size());
  for(uint i=0; i < colors.size(); ++i){
    qp.setColor(QPalette::Active, QPalette::WindowText, colors[i]);
    qp.setColor(QPalette::Inactive, QPalette::WindowText, colors[i]);
    labels[i] = new QLabel("---", this);
    labels[i]->setPalette(qp);
    grid->addWidget(labels[i], i / maxColNo, i % maxColNo);
    //    hbox->addWidget(labels[i]);
  }  
}


void ValueLabels::setValues(vector<float> v)
{
  QString str;
  for(uint i=0; i < v.size() && i < labels.size(); ++i){
    setValue(i, v[i]);
  }
}

void ValueLabels::setValue(unsigned int i, float v)
{
  if(i >= labels.size())
    return;
  QString str;
  str.sprintf("%1.2e", v);
  labels[i]->setText(str);
}

void ValueLabels::setColor(unsigned int i, QColor c)
{
  if(i >= labels.size())
    return;
  QPalette qp(palette());
  qp.setColor(QPalette::Active, QPalette::WindowText, c);
  qp.setColor(QPalette::Inactive, QPalette::WindowText, c);
  labels[i]->setPalette(qp);
}

#include "backgroundWidget.h"
#include <QHBoxLayout>
#include <QTextStream>
#include <QString>
#include <QLabel>

BackgroundWidget::BackgroundWidget(fluorInfo flinfo, QWidget* parent)
  : QWidget(parent)
{
  fluor = flinfo;
  
  // make some labels.
  QString label;
  QTextStream ts(&label);
  ts << fluor.excitation << " --> " << fluor.emission << endl;
  

  QLabel* chLabel = new QLabel(label.latin1(), this);
  //chLabel->setAlignment(Qt::AlignLeft|Qt::AlignBottom);
  QLabel* xlabel = new QLabel("x_m", this);
  QLabel* ylabel = new QLabel("y_m", this);
  QLabel* zlabel = new QLabel("z_m", this);
  QLabel* plabel = new QLabel("pcntile", this);
  
  // Some SpinBoxes for values
  xmv = new QSpinBox(this);
  ymv = new QSpinBox(this);
  zmv = new QSpinBox(this);
  pcv = new QSpinBox(this);

  // set the values for the boxes..
  xmv->setRange(2, 256);
  ymv->setRange(2, 256);
  zmv->setRange(2, 32);  // it would be better to use values on the basis of known proportions,
  pcv->setRange(1, 99); // 100 would mxake it black.

  // set some reasonable values
  xmv->setValue(32);
  ymv->setValue(32);
  zmv->setValue(8);
  pcv->setValue(50);

  // and a simple layout
  QHBoxLayout* hbox = new QHBoxLayout(this);
  hbox->addWidget(chLabel);
  hbox->addWidget(xlabel);
  hbox->addWidget(xmv);
  hbox->addWidget(ylabel);
  hbox->addWidget(ymv);
  hbox->addWidget(zlabel);
  hbox->addWidget(zmv);
  hbox->addWidget(plabel);
  hbox->addWidget(pcv);
}

backgroundPars BackgroundWidget::pars(){
  backgroundPars bgp(xmv->value(), ymv->value(), zmv->value(), (float)pcv->value());
  return(bgp);
}

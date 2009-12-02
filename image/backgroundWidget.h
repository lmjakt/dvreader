#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include "../dataStructs.h"
#include <QSpinBox>

class BackgroundWidget : public QWidget
{
  Q_OBJECT
    public :
  BackgroundWidget(fluorInfo flinfo, QWidget* parent);
  
  backgroundPars pars();

 private :
  QSpinBox* xmv;
  QSpinBox* ymv;
  QSpinBox* zmv;
  QSpinBox* pcv;
  fluorInfo fluor;
};


#endif

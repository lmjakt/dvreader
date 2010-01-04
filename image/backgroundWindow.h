#ifndef BACKGROUNDWINDOW_H
#define BACKGROUNDWINDOW_H

#include "backgroundWidget.h"
#include "../dataStructs.h"
#include <map>
#include <set>

class BackgroundWindow : public QWidget
{
  Q_OBJECT

    public :
  BackgroundWindow(std::set<fluorInfo> fli, QWidget* parent=0);

 signals :
  void setBackgroundPars(std::map<fluorInfo, backgroundPars> );

 private slots :
  void setPars();

 private :
  std::map<fluorInfo, BackgroundWidget*> backgroundWidgets;
};  

#endif

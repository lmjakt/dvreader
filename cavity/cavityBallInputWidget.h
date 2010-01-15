#ifndef CAVITYBALLINPUTWIDGET_H
#define CAVITYBALLINPUTWIDGET_H


#include <QWidget>
#include <vector>
#include "../spotFinder/channelSelector.h"
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QString>

class CavityBallInputWidget : public QWidget
{
  Q_OBJECT

    public:
  CavityBallInputWidget(QWidget* parent=0);
  void setChannels(std::vector<QString> channels);
  
 private:
  ChannelSelector* chSelector;
  QSpinBox* xRadius;
  QSpinBox* yRadius;
  QSpinBox* zRadius;
  QDoubleSpinBox* maxP;
  QDoubleSpinBox* maxDP;
  
  QSpinBox* makeSpinBox(QString suff, int min, int max);
  QDoubleSpinBox* makeDoubleSpinBox(QString suff, float min, float max, float step);

  private slots:
  void mapCavities();

 signals:
  void mapCavities(int, int, int, int, float, float);
  
};

#endif

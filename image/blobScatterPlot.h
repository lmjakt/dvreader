#ifndef BLOBSCATTERPLOT_H
#define BLOBSCATTERPLOT_H

#include "../scatterPlot/scatterPlotter.h"
#include "blobMapperWidget.h"
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <vector>

class BlobScatterPlot : public QWidget
{
  Q_OBJECT
    public:
  BlobScatterPlot(QWidget* parent=0);
  
  void setData(std::vector<std::vector<float> > xv, std::vector<std::vector<float> > yv, std::vector<QColor> c);
  BlobMapperWidget::Param x_param();
  BlobMapperWidget::Param y_param();

 signals :
  void plotPars(BlobMapperWidget::Param, BlobMapperWidget::Param);
 void blobsSelected(std::vector<std::vector<bool> >);

  private slots:
  void changePlotParams(int p);
  void changeSelection();
  
  void filterBlobs();   // removes blobs in the painter path
  void selectBlobs();   // selects blobs in the painter path

 private:
  ScatterPlotter* plotter;
  QComboBox* xParam;
  QComboBox* yParam;
  QSpinBox* alphaBox;
  QHBoxLayout* hbox;
  std::vector<QCheckBox*> colorBoxes;

  void setParams(QComboBox* box);
  BlobMapperWidget::Param getParam(QComboBox* box);
  void selectBlobs(bool plotBlob);  // if plotBlob select, otherwise filter.. 

};

#endif

#ifndef BLOBSCATTERPLOT_H
#define BLOBSCATTERPLOT_H

#include "../scatterPlot/scatterPlotter.h"
#include "blobMapper.h"
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
  BlobMapper::Param x_param();
  BlobMapper::Param y_param();

 signals :
  void plotPars(BlobMapper::Param, BlobMapper::Param);
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
  BlobMapper::Param getParam(QComboBox* box);
  void selectBlobs(bool plotBlob);  // if plotBlob select, otherwise filter.. 

};

#endif

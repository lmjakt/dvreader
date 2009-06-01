#ifndef BLOBMAPPERWIDGETMANAGER_H
#define BLOBMAPPERWIDGETMANAGER_H

#include "blobMapperWidget.h"
#include "superBlob.h"
#include "../linGraph/distPlotter.h"
#include "blobScatterPlot.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QColor>
#include <set>
#include <vector>
#include <string>

class BlobMapperWidgetManager : public QWidget
{
    Q_OBJECT
      public:
    BlobMapperWidgetManager(QWidget* parent=0);
    ~BlobMapperWidgetManager();

    void addBlobMapper(BlobMapper* bm, fluorInfo& fInfo, std::string fName, QColor c);
    std::set<BlobMapperWidget*> blobMapperWidgets();

 signals:
    void newColor();
    void newRep();
    void newLimits();

    private slots:
	void setParamType(int p);
    void deleteBlobWidget();
    void makeSuperBlobs();
    void exportSuperBlobs();
    void replot();
    void clearPlotLimits();
    void setLimits(float l, float r);
    void filterBlobs(std::vector<std::vector<bool> > blobSelection);
    void scatterPlot(BlobMapperWidget::Param xpar, BlobMapperWidget::Param ypar);

 private:
    std::set<BlobMapperWidget*> blobWidgets;
    std::vector<SuperBlob*> superBlobs;
    //  QComboBox* blobTypeSelector;
    DistPlotter* distPlotter;
    DistPlotter* superDistPlotter;
    BlobMapperWidget::Param plotType;
    BlobScatterPlot* scatterPlotter;
    QVBoxLayout* vbox;

    void plotDistributions();
    void collectSuperBlobValues(std::vector<std::vector<float > >& plotValues, 
				std::vector<QColor>& plotColors, pl_limits& plotLimits,
				BlobMapperWidget::Param p_type);
    std::map<unsigned int, std::vector<blob*> > collectSuperBlobBlobs();
    std::map<unsigned int, bool> distIncludeMap();
    std::vector<QColor> widgetPlotColors();
    pl_limits get_plotLimits(BlobMapperWidget::Param p_type);
    void plotSuperDistributions();
    void deleteSuperBlobs();
};

#endif
